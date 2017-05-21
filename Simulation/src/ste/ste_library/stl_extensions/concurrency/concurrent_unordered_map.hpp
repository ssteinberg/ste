// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <atomic>
#include <memory>

#include <functional>
#include <type_traits>

#include <array>
#include <vector>
#include <bitset>

#include <aligned_allocator.hpp>

#include <shared_double_reference_guard.hpp>

namespace ste {

/**
 *	@brief	Concurrent, lock-free, unordered key-value hashmap.
 *			Provides insert/replace, erase and automatic expand facilities.
 *			
 *			Key and value types must be copyable. 
 *			Expects an aligned allocator that aligns on cache_line boundaries. AlignedAllocator and Allocator are expected to be stateless.
 */
template <
	typename K, 
	typename V,
	int cache_line = 64,
	typename AlignedAllocator = aligned_allocator<V, cache_line>,
	typename Allocator = std::allocator<V>
>
class concurrent_unordered_map {
private:
	using Hasher = std::hash<K>;
	template <typename T>
	using double_ref_guard = shared_double_reference_guard<T, typename Allocator::template rebind<T>::other>;

public:
	using mapped_type = V;
	using key_type = K;
	using allocator_type = Allocator;

private:
	struct concurrent_map_bucket_data {
		using value_type = double_ref_guard<mapped_type>;
		using value_data_guard_type = typename value_type::data_guard;

		key_type k;
		value_type *v;

		template <typename S, typename ... Ts>
		concurrent_map_bucket_data(S&& k, Ts&&... args) 
			: k(std::forward<S>(k))
		{
			v = Allocator::template rebind<value_type>::other().allocate(1);
			::new (v) value_type(std::forward<Ts>(args)...);
		}
		~concurrent_map_bucket_data() {
			v->~value_type();
			Allocator::template rebind<value_type>::other().deallocate(v, 1);
		}
	};

	static_assert(std::is_default_constructible<mapped_type>::value, "V must be default constructible.");

	using hash_type = std::conditional_t<sizeof(void*) == 4, std::uint32_t, std::uint64_t>;

	static constexpr int bucket_size = sizeof(hash_type) + sizeof(concurrent_map_bucket_data*);

	static_assert(sizeof(hash_type) == sizeof(concurrent_map_bucket_data*), "Sanity check error");
	static_assert((cache_line % bucket_size) == 0, "cache_line is indivisible by bucket size.");

	static constexpr int N = 2 * cache_line / bucket_size - 1;

	static_assert(N > 2, "cache_line can't hold enough buckets");

	static constexpr int depth_threshold = 1;
	static constexpr float min_load_factor_for_resize = .5f;

	// Arrange all the hashes together with the next pointer in a single cache line, and place the buckets in the next line. This allows us to search a whole 
	// virtual bucket (7 elements on x86-64, 15 on x86) as well as access the next pointer without cache misses.
	struct concurrent_map_virtual_bucket {
		std::array<std::atomic<hash_type>, N> hash;
		std::atomic<concurrent_map_virtual_bucket*> next{ nullptr };

		std::array<std::atomic<concurrent_map_bucket_data*>, N> buckets;

		void *__unused;

		static_assert(N * sizeof(hash_type) == sizeof(decltype(hash)));
		static_assert(N * sizeof(concurrent_map_bucket_data*) == sizeof(decltype(buckets)));
		static_assert(sizeof(decltype(buckets)) == sizeof(decltype(hash)));
		static_assert(sizeof(decltype(next)) == sizeof(decltype(__unused)));

		concurrent_map_virtual_bucket() {
			std::fill(std::begin(hash), std::end(hash), 0);
			std::fill(std::begin(buckets), std::end(buckets), nullptr);
		}
		~concurrent_map_virtual_bucket() {
			{
				auto ptr = next.load();
				if (ptr) {
					ptr->~concurrent_map_virtual_bucket();
					Allocator::template rebind<concurrent_map_virtual_bucket>::other().deallocate(ptr, 1);
				}
			}
			for (auto &b : buckets) {
				auto ptr = b.load();
				if (ptr) {
					ptr->~concurrent_map_bucket_data();
					Allocator::template rebind<concurrent_map_bucket_data>::other().deallocate(ptr, 1);
				}
			}
		}
	};

	static_assert(2 * cache_line == sizeof(concurrent_map_virtual_bucket), "Nonsensical padding on concurrent_map_virtual_bucket");

	using virtual_bucket_type = concurrent_map_virtual_bucket;
	using hash_table_type = virtual_bucket_type*;

	template <class table_ptr>
	struct resize_data_struct {
		using marker_t = std::uint8_t;
		static constexpr std::size_t chunk_size = 8;

		unsigned size;
		hash_table_type buckets;
		typename double_ref_guard<table_ptr>::data_guard new_table_guard{ 0 };
		std::vector<std::atomic<marker_t>> markers;

		resize_data_struct(unsigned size, unsigned old_size) : size(size), buckets(table_ptr::alloc(size)), markers((old_size + chunk_size - 1) / chunk_size) {
			assert(markers[0].is_lock_free() && "markers not lock free");
		}
	};

	struct buckets_ptr {
		unsigned size;
		hash_table_type buckets{ nullptr };
		double_ref_guard<resize_data_struct<buckets_ptr>> resize_ptr;

		static hash_table_type alloc(unsigned size) {
			auto b = AlignedAllocator::template rebind<virtual_bucket_type>::other().allocate(size);
			for (unsigned i=0;i<size;++i)
				new (b + i) virtual_bucket_type();

			assert(reinterpret_cast<std::size_t>(b) % static_cast<std::size_t>(cache_line) == 0 && "AlignedAllocator does not aligne properly!");
			assert(b[0].buckets[0].is_lock_free() && "bucket_type not lock free");

			return b;
		}

		void dealloc_buckets() {
			if (!buckets)
				return;

			for (auto i = size; i-->0;)
				(&buckets[i])->~virtual_bucket_type();
			AlignedAllocator::template rebind<virtual_bucket_type>::other().deallocate(buckets, size);
		}

		buckets_ptr(unsigned size, hash_table_type table) : size(size), buckets(table) {}
		buckets_ptr(unsigned size) : size(size), buckets(alloc(size)) {}
		~buckets_ptr() {
			resize_ptr.drop();
		}

		buckets_ptr(const buckets_ptr &b) = delete;
		buckets_ptr &operator=(const buckets_ptr &b) = delete;
	};

	using resize_data = resize_data_struct<buckets_ptr>;
	using resize_data_guard_type = typename double_ref_guard<resize_data>::data_guard;
	using hash_table_guard_type = typename double_ref_guard<buckets_ptr>::data_guard;

public:
	using value_data_guard_type = typename concurrent_map_bucket_data::value_data_guard_type;

private:
	mutable double_ref_guard<buckets_ptr> hash_table;
	std::atomic<int> items{ 0 };

	template <typename T>
	hash_type hash_function(T&& t) const { auto h = Hasher()(std::forward<T>(t)); return h ? h : 1; }

	template <typename T>
	void copy_to_new_table(resize_data_guard_type &resize_guard, hash_type hash, const key_type &key, T&& val) {
		hash_type mask = resize_guard->size - 1;
		hash_type i = hash & mask;
		auto &virtual_bucket = resize_guard->buckets[i];

		insert_update_into_virtual_bucket(virtual_bucket, hash, key, .0f, false, 1, std::forward<T>(val));
	}

	void copy_virtual_bucket(hash_table_guard_type &old_table_guard,
							 resize_data_guard_type &resize_guard,
							 std::size_t i, std::size_t chunks) {
		std::size_t chunk_size = resize_data::chunk_size;
		auto j = i * chunk_size;
		for (std::size_t k = 0; k < chunk_size && j < old_table_guard->size; ++k, ++j) {
			auto *virtual_bucket = &old_table_guard->buckets[j];
			for (;;) {
				for (int b = 0; b < N; ++b) {
					auto bucket = virtual_bucket->buckets[b].load();
					if (bucket) {
						auto val_guard = bucket->v->acquire();
						if (val_guard.is_valid())
							copy_to_new_table(resize_guard, virtual_bucket->hash[b].load(), bucket->k, *val_guard);
						else
							items.fetch_sub(1, std::memory_order_relaxed);
					}
				}

				virtual_bucket = virtual_bucket->next.load();
				if (!virtual_bucket)
					break;
			}
		}
	}

	template <typename ... Ts>
	void resize_with_pending_insert(hash_table_guard_type &old_table_guard,
									resize_data_guard_type &resize_guard,
									hash_type hash,
									const key_type &key,
									bool helper_only,
									bool delete_item,
									Ts&&... val_args) {
		if (!resize_guard.is_valid())
			resize_guard = old_table_guard->resize_ptr.acquire();
		if (!resize_guard.is_valid()) {
			if (helper_only)
				return;
			unsigned new_size = 2 * old_table_guard->size;
			while (!resize_guard.is_valid()) {
				old_table_guard->resize_ptr.try_compare_emplace(std::memory_order_acq_rel, resize_guard, new_size, old_table_guard->size);
				resize_guard = old_table_guard->resize_ptr.acquire();
				assert(resize_guard.is_valid());
			}
		}

		hash_type mask = resize_guard->size - 1;
		hash_type j = hash & mask;
		auto &virtual_bucket = resize_guard->buckets[j];
		!delete_item ?
			insert_update_into_virtual_bucket(virtual_bucket, hash, key, .0f, true, 1, std::forward<Ts>(val_args)...) :
			remove_from_virtual_bucket(virtual_bucket, hash, key);

		auto chunks = resize_guard->markers.size();
		for (std::size_t i = 0; i < chunks; ++i) {
			typename resize_data::marker_t old_val = 0;
			if (!resize_guard->markers[i].compare_exchange_strong(old_val, 1, std::memory_order_acq_rel, std::memory_order_relaxed))
				continue;
			copy_virtual_bucket(old_table_guard, resize_guard, i, chunks);
			resize_guard->markers[i].store(2, std::memory_order_release);
		}

		bool old_table_moved = true;
		for (std::size_t i = 0; i < chunks; ++i)
			if (!(old_table_moved &= resize_guard->markers[i].load(std::memory_order_acquire) == 2)) break;
		if (old_table_moved) {
			if (hash_table.try_compare_emplace(std::memory_order_acq_rel, old_table_guard, resize_guard->size, resize_guard->buckets)) {
				old_table_guard->dealloc_buckets();
			}
		}
	}

	int find_hash_in_virtual_bucket(virtual_bucket_type *virtual_bucket, hash_type hash) const {
		for (int j = 0; j < N; ++j) {
			auto h = virtual_bucket->hash[j].load(std::memory_order_relaxed);
			if (h == hash)
				return j;
		}
		return -1;
	}

	bool remove_from_virtual_bucket(concurrent_map_virtual_bucket &virtual_bucket, hash_type hash, const key_type &key) {
		int pos = find_hash_in_virtual_bucket(&virtual_bucket, hash);
		if (pos >= 0) {
			auto bucket = virtual_bucket.buckets[pos].load();
			if (bucket && bucket->k == key) {
				bucket->v->drop();
				return true;
			}
		}

		auto next_ptr = virtual_bucket.next.load();
		if (!next_ptr)
			return false;
		return remove_from_virtual_bucket(*next_ptr, hash, key);
	}

	template <typename ... Ts>
	bool insert_update_into_virtual_bucket(concurrent_map_virtual_bucket &virtual_bucket,
										   hash_type hash,
										   const key_type &key,
										   float load_factor,
										   bool is_new_item_insert,
										   int depth,
										   Ts&&... val_args) {
		bool request_resize = false;
		for (int j = 0; j < N;) {
			auto bucket_data = virtual_bucket.buckets[j].load();
			if (!bucket_data) {
				hash_type old_hash = virtual_bucket.hash[j].load(std::memory_order_relaxed);
				if (virtual_bucket.hash[j].compare_exchange_strong(old_hash, hash, std::memory_order_acq_rel, std::memory_order_relaxed)) {
					auto bucket = Allocator::template rebind<concurrent_map_bucket_data>::other().allocate(1);
					::new (bucket) concurrent_map_bucket_data(key, std::forward<Ts>(val_args)...);
					virtual_bucket.buckets[j].store(bucket, std::memory_order_release);

					if (is_new_item_insert)
						items.fetch_add(1, std::memory_order_relaxed);

					return !request_resize;
				}

				continue;
			}

			hash_type old_hash = virtual_bucket.hash[j].load();
			if (old_hash != hash || bucket_data->k != key) {
				++j; continue;
			}
			if (!is_new_item_insert)
				return false;
			bucket_data->v->emplace(std::memory_order_release, std::forward<Ts>(val_args)...);
			return !request_resize;
		}

		if (load_factor >= min_load_factor_for_resize && depth >= depth_threshold)
			request_resize = true;

		auto next_ptr = virtual_bucket.next.load();
		while (!next_ptr) {
			auto new_next = Allocator::template rebind<virtual_bucket_type>::other().allocate(1);
			::new (new_next) virtual_bucket_type();

			if (virtual_bucket.next.compare_exchange_strong(next_ptr, new_next, std::memory_order_acq_rel, std::memory_order_acquire))
				next_ptr = new_next;

			assert(next_ptr);
		}

		return !request_resize && insert_update_into_virtual_bucket(*next_ptr, hash, key, load_factor, is_new_item_insert, depth + 1, std::forward<Ts>(val_args)...);
	}

public:
	concurrent_unordered_map() : hash_table(1024) {}
	~concurrent_unordered_map() {
		auto data_guard = hash_table.acquire();
		data_guard->dealloc_buckets();
	}

	template <typename ... Ts>
	void emplace(const key_type &key, Ts&&... val_args) {
		hash_type hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		hash_type mask = table_guard->size - 1;
		hash_type i = hash & mask;
		auto &virtual_bucket = table_guard->buckets[i];

		resize_data_guard_type resize_guard{ 0 };
		if (!insert_update_into_virtual_bucket(virtual_bucket,
											   hash, key,
											   static_cast<float>(items.load(std::memory_order_relaxed)) / static_cast<float>(table_guard->size * N),
											   true, 1,
											   std::forward<Ts>(val_args)...)) {
			resize_with_pending_insert(table_guard, resize_guard, hash, key, false, false, std::forward<Ts>(val_args)...);
			return;
		}

		resize_guard = table_guard->resize_ptr.acquire();
		if (resize_guard.is_valid())
			resize_with_pending_insert(table_guard, resize_guard, hash, key, true, false, std::forward<Ts>(val_args)...);
	}

	void remove(const key_type &key) {
		hash_type hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		hash_type mask = table_guard->size - 1;
		hash_type i = hash & mask;
		remove_from_virtual_bucket(table_guard->buckets[i], hash, key);

		auto resize_guard = table_guard->resize_ptr.acquire();
		if (resize_guard.is_valid())
			resize_with_pending_insert(table_guard, resize_guard, hash, key, true, true);
	}

	value_data_guard_type try_get(const key_type &key) const {
		hash_type hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		hash_type mask = table_guard->size - 1;
		hash_type i = hash & mask;

		auto *virtual_bucket = &table_guard->buckets[i];
		for (;;) {
			int pos = find_hash_in_virtual_bucket(virtual_bucket, hash);
			if (pos >= 0) {
				auto bucket = virtual_bucket->buckets[pos].load(std::memory_order_relaxed);
				if (bucket && bucket->k == key)
					return bucket->v->acquire(std::memory_order_relaxed);
			}

			virtual_bucket = virtual_bucket->next.load(std::memory_order_relaxed);
			if (!virtual_bucket)
				return value_data_guard_type(0);
		}
	}

	decltype(auto) operator[](const key_type &k) const {
		return try_get(k);
	}
};

}
