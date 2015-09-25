// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <atomic>
#include <memory>
#include <functional>
#include <array>
#include <vector>
#include <bitset>

#include <stdlib.h>

#include "atomic_double_ref_guard.h"

namespace StE {

template <typename K, typename V, int cache_line = 64>
class concurrent_unordered_map {
private:
	using Hasher = std::hash<K>;

public:
	using mapped_type = V;
	using key_type = K;

private:
	struct concurrent_map_bucket_data {
		key_type k;
		atomic_double_ref_guard<mapped_type> v;

		template <typename S, typename T>
		concurrent_map_bucket_data(S&& k, T&& v) : k(std::forward<S>(k)), v(std::forward<T>(v)) {}
	};

	struct concurrent_map_bucket_ptr {
		unsigned hash;
		concurrent_map_bucket_data *data;

		bool is_valid() const { return !!data; }
	};

	using bucket_type = std::atomic<concurrent_map_bucket_ptr>;

	static constexpr int N = cache_line / sizeof(bucket_type) - 1;
	static constexpr int depth_threshold = 2;
	static constexpr float min_load_factor_for_resize = .5f;

	static_assert((cache_line % sizeof(bucket_type)) == 0, "cache_line is indivisible by sizeof(bucket_type)");
	static_assert(N > 2, "cache_line can't hold enough buckets");

	struct concurrent_map_virtual_bucket {
		std::array<bucket_type, N> buckets{ concurrent_map_bucket_ptr{ 0,nullptr } };
		std::atomic<concurrent_map_virtual_bucket*> next{ nullptr };
		void *_unused;

		~concurrent_map_virtual_bucket() {
			auto ptr = next.load();
			if (ptr) 
				delete ptr;
			for (bucket_type &b : buckets) {
				auto ptr = b.load().data;
				if (ptr) delete ptr;
			}
		}
	};

	static_assert(cache_line == sizeof(concurrent_map_virtual_bucket), "Nonsensical padding on concurrent_map_virtual_bucket");

	using virtual_bucket_type = concurrent_map_virtual_bucket;
	using hash_table_type = virtual_bucket_type*;

	template <class table_ptr>
	struct resize_data_struct {
		static constexpr int chunk_size = 8;

		unsigned size;
		hash_table_type buckets;
		atomic_double_ref_guard<table_ptr>::data_guard new_table_guard{ nullptr };
		std::vector<std::atomic<int>> markers;

		resize_data_struct(unsigned size, unsigned old_size) : size(size), buckets(table_ptr::alloc(size)), markers((old_size + chunk_size - 1) / chunk_size) {
			assert(markers[0].is_lock_free() && "markers not lock free");
		}
	};

	struct buckets_ptr {
		unsigned size;
		hash_table_type buckets{ nullptr };
		atomic_double_ref_guard<resize_data_struct<buckets_ptr>> resize_ptr;
		bool destructable{ true };

		static hash_table_type alloc(unsigned size) {
			auto b = reinterpret_cast<virtual_bucket_type*>(_aligned_malloc(sizeof(virtual_bucket_type) * size, cache_line));
			new (b) virtual_bucket_type[size];
			assert(b[0].buckets[0].is_lock_free() && "bucket_type not lock free");
			return b;
		}

		buckets_ptr(unsigned size, hash_table_type table) : size(size), buckets(table), destructable(false) {}
		buckets_ptr(unsigned size) : size(size), buckets(alloc(size)) {}
		~buckets_ptr() { destroy(); }
		void destroy() {
			if (buckets && destructable) {
				for (int i = 0; i < size; ++i)
					(&buckets[i])->~virtual_bucket_type();
				_aligned_free(buckets);
			}
		}

		buckets_ptr(const buckets_ptr &b) = delete;
		buckets_ptr &operator=(const buckets_ptr &b) = delete;
	};

	using resize_data = resize_data_struct<buckets_ptr>;

private:
	atomic_double_ref_guard<buckets_ptr> hash_table;
	std::atomic<int> items{ 0 };

	template <typename T>
	unsigned hash_function(T&& t) const { return Hasher()(std::forward<T>(t)); }

	void copy_to_new_table(atomic_double_ref_guard<resize_data>::data_guard &resize_guard, unsigned hash, const key_type &key, const mapped_type &val) {
		unsigned i = hash % resize_guard->size;
		auto &virtual_bucket = resize_guard->buckets[i];

		insert_update_into_virtual_bucket(virtual_bucket, hash, key, val, .0f, false);
	}

	void copy_virtual_bucket(atomic_double_ref_guard<buckets_ptr>::data_guard &old_table_guard,
							 atomic_double_ref_guard<resize_data>::data_guard &resize_guard,
							 int i, int chunks) {
		int chunk_size = resize_data::chunk_size;
		int j = i * chunk_size;
		for (int k = 0; k < chunk_size && j < old_table_guard->size; ++k, ++j) {
			auto *virtual_bucket = &old_table_guard->buckets[j];
			for (;;) {
				for (int b = 0; b < N; ++b) {
					auto bucket = virtual_bucket->buckets[b].load();
					if (bucket.is_valid()) {
						auto val_guard = bucket.data->v.acquire();
						val_guard.is_valid() ?
							copy_to_new_table(resize_guard, bucket.hash, bucket.data->k, *val_guard) :
							items.fetch_sub(1, std::memory_order_relaxed);
					}
				}

				virtual_bucket = virtual_bucket->next.load();
				if (!virtual_bucket)
					break;
			}
		}
	}

	void resize_with_pending_insert(atomic_double_ref_guard<buckets_ptr>::data_guard &old_table_guard, 
									atomic_double_ref_guard<resize_data>::data_guard &resize_guard,
									unsigned hash, 
									const key_type &key, 
									const mapped_type *val, 
									bool helper_only = true) {
		if (!resize_guard.is_valid())
			resize_guard = old_table_guard->resize_ptr.acquire();
		if (!resize_guard.is_valid()) {
			if (helper_only)
				return;
			unsigned new_size = 2 * old_table_guard->size + 3;
			while (!resize_guard.is_valid()) {
				old_table_guard->resize_ptr.try_compare_emplace(std::memory_order_acq_rel, resize_guard, new_size, old_table_guard->size);
				resize_guard = old_table_guard->resize_ptr.acquire();
				assert(resize_guard.is_valid());
			}
		}

		int chunks = resize_guard->markers.size();
		for (int i = 0; i < chunks; ++i) {
			int old_val = 0;
			if (!resize_guard->markers[i].compare_exchange_strong(old_val, 1, std::memory_order_acq_rel, std::memory_order_relaxed))
				continue;
			copy_virtual_bucket(old_table_guard, resize_guard, i, chunks);
			resize_guard->markers[i].store(2, std::memory_order_release);
		}

		bool old_table_moved = true;
		for (int i = 0; i < chunks; ++i)
			if (!(old_table_moved &= resize_guard->markers[i].load(std::memory_order_acquire) == 2)) break;
		if (old_table_moved) {
			hash_table.try_compare_emplace(std::memory_order_acq_rel, old_table_guard, resize_guard->size, resize_guard->buckets);
			hash_table.acquire()->destructable = true;
			old_table_guard->resize_ptr.drop();
		}

		auto table_guard = hash_table.acquire();
		unsigned i = hash % table_guard->size;
		auto &virtual_bucket = table_guard->buckets[i];

		val ?
			insert_update_into_virtual_bucket(virtual_bucket, hash, key, *val) :
			remove_from_virtual_bucket(virtual_bucket, hash, key);
	}

	bool remove_from_virtual_bucket(concurrent_map_virtual_bucket &virtual_bucket, unsigned hash, const key_type &key) {
		for (int j = 0; j < N; ++j) {
			auto &bucket = virtual_bucket.buckets[j];
			auto bucket_ptr = bucket.load();
			if (bucket_ptr.is_valid()) {
				if (bucket_ptr.hash == hash && bucket_ptr.data->k == key) {
					bucket_ptr.data->v.drop();
					return true;
				}
			}
		}

		auto next_ptr = virtual_bucket.next.load();
		if (!next_ptr)
			return false;
		return remove_from_virtual_bucket(*next_ptr, hash, key);
	}

	bool insert_update_into_virtual_bucket(concurrent_map_virtual_bucket &virtual_bucket, 
										   unsigned hash, 
										   const key_type &key, 
										   const mapped_type &val,
										   float load_factor = .0f,
										   bool new_item_insert = true,
										   int depth = 1) {
		for (int j = 0; j < N;) {
			auto &bucket = virtual_bucket.buckets[j];
			auto bucket_ptr = bucket.load();
			if (!bucket_ptr.is_valid()) {
				concurrent_map_bucket_ptr new_bucket_data;
				new_bucket_data.hash = hash;
				auto ptr = new_bucket_data.data = new concurrent_map_bucket_data(key, val);

				if (bucket.compare_exchange_strong(bucket_ptr, new_bucket_data, std::memory_order_acq_rel, std::memory_order_relaxed)) {
					if (new_item_insert)
						items.fetch_add(1, std::memory_order_relaxed);
					return true;
				}

				delete ptr;
				continue;
			}

			if (bucket_ptr.hash != hash || bucket_ptr.data->k != key) {
				++j; continue;
			}
			if (!new_item_insert)
				return false;
			bucket_ptr.data->v.emplace(std::memory_order_acq_rel, val);
			return true;
		}

		if (depth >= depth_threshold && load_factor >= min_load_factor_for_resize)
			return false;

		auto next_ptr = virtual_bucket.next.load();
		while (!next_ptr) {
			auto new_next = new virtual_bucket_type;
			if (virtual_bucket.next.compare_exchange_strong(next_ptr, new_next, std::memory_order_acq_rel, std::memory_order_acquire))
				next_ptr = new_next;
			assert(next_ptr);
		}

		return insert_update_into_virtual_bucket(*next_ptr, hash, key, val, load_factor, new_item_insert, depth + 1);
	}

public:
	concurrent_unordered_map(std::size_t count = 1019) : hash_table(count) {}

	void insert(const key_type &key, const mapped_type &val) {
		unsigned hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		unsigned i = hash % table_guard->size;
		auto &virtual_bucket = table_guard->buckets[i];

		atomic_double_ref_guard<resize_data>::data_guard resize_guard{ nullptr };
		if (!insert_update_into_virtual_bucket(virtual_bucket,
											   hash, key, val,
											   static_cast<float>(items.load(std::memory_order_relaxed)) / static_cast<float>(table_guard->size * N))) {
			resize_with_pending_insert(table_guard, resize_guard, hash, key, &val, false);
			return;
		}

		resize_guard = table_guard->resize_ptr.acquire();
		if (resize_guard.is_valid())
			resize_with_pending_insert(table_guard, resize_guard, hash, key, &val);
	}

	void remove(const key_type &key) {
		unsigned hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		unsigned i = hash % table_guard->size;
		remove_from_virtual_bucket(table_guard->buckets[i], hash, key);

		auto resize_guard = table_guard->resize_ptr.acquire();
		if (resize_guard.is_valid())
			resize_with_pending_insert(table_guard, resize_guard, hash, key, nullptr);
	}

	bool try_get(const key_type &key, mapped_type &out) {
		unsigned hash = hash_function(key);

		auto table_guard = hash_table.acquire();
		unsigned i = hash % table_guard->size;

		auto *virtual_bucket = &table_guard->buckets[i];
		for (;;) {
			for (int j = 0; j < N; ++j) {
				auto bucket = virtual_bucket->buckets[j].load(std::memory_order_relaxed);
				if (bucket.is_valid()) {
					if (bucket.hash == hash && bucket.data->k == key) {
						auto val_guard = bucket.data->v.acquire();
						if (!val_guard.is_valid())
							return false;
						out = *val_guard;
						return true;
					}
				}
			}

			virtual_bucket = virtual_bucket->next.load(std::memory_order_relaxed);
			if (!virtual_bucket)
				return false;
		}
	}
};

}
