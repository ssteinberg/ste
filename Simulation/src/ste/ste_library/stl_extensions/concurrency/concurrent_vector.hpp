// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <memory>
#include <atomic>

#include <shared_double_reference_guard.hpp>

#include <initializer_list>
#include <vector>

namespace ste {

template <typename T, typename Allocator = std::allocator<T>>
class concurrent_vector {
public:
	using element_type = shared_double_reference_guard<T, Allocator>;
	using allocator_type = Allocator;
	using storage_allocator_type = typename Allocator::template rebind<element_type>::other;

	using value_type = typename allocator_type::value_type;
	using reference = typename allocator_type::reference;
	using const_reference = typename allocator_type::const_reference;
	using pointer = typename allocator_type::pointer;
	using const_pointer = typename allocator_type::const_pointer;
	using difference_type = typename allocator_type::difference_type;
	using size_type = typename allocator_type::size_type;

private:
	template <typename >
	struct resize_data {
		static constexpr std::uint8_t marker_unclaimed = 0;
		static constexpr std::uint8_t marker_in_progress = 1;
		static constexpr std::uint8_t marker_completed = 2;

		concurrent_vector *vec;
		typename root_t::data_guard &old_root;
		std::vector<std::atomic<std::uint8_t>> markers;
		std::atomic<std::uint8_t> resize_complete{ marker_unclaimed };

		data new_data;
		std::size_t old_size;

		resize_data(std::size_t new_size, typename root_t::data_guard &old_root, concurrent_vector *vec)
			: old_root(old_root), markers(new_size, marker_unclaimed), new_data(new_size), old_size(old_root->size.load())
		{}

		void join() {
			// Transfer elements to new vector
			std::size_t start = 0;
			for (std::size_t i = start; i<start + old_size; ++i) {
				auto idx = i % old_size;

				// Try to claim chunk
				auto expected = marker_unclaimed;
				if (!markers[idx].compare_exchange_strong(expected, marker_in_progress))
					continue;

				// Claimed successfuly
				auto old_guard = (old_root->storage + idx)->acquire();
				if (old_guard.is_valid()) {
					// Copy to new vector
					(new_data.storage + idx)->emplace(*old_guard);
				}

				// Make completed
				markers[idx].store(marker_completed, std::memory_order_release);
			}

			// Wait for all transfers to finish
			for (std::size_t i = 0; i<old_size; ++i) {
				while (markers[idx])
			}
		}
	};

	struct data {
		element_type* storage;
		std::atomic<std::size_t> size;
		std::size_t capacity;

		shared_double_reference_guard<resize_data, Allocator<resize_data>> resize;

		storage_allocator_type storage_allocator;

		data(data&& o, std::size_t size) noexcept : storage(o.storage), size(size), capacity(o.capacity), resize(nullptr), storage_allocator(std::move(o.storage_allocator)) {}

		data(std::size_t count) : size(0), capacity(count) {
			storage = storage_allocator.allocate(count);
			for (std::size_t i = 0; i < count; ++i)
				::new (storage + i) element_type();
		}
		template <typename... Args>
		data(std::size_t count, Args&&... args) : size(0), capacity(count) {
			storage = storage_allocator.allocate(count);
			for (std::size_t i = 0; i < count; ++i)
				::new (storage + i) element_type(std::forward<Args>(args)...);
		}
		~data() noexcept {
			for (std::size_t i = capacity; i-->0;)
				(storage + i)->~element_type();
			storage_allocator.deallocate(storage, capacity);
		}
	};

	using root_t = shared_double_reference_guard<data, storage_allocator_type>;
	static constexpr auto initial_count = 8;

private:
	template <typename value_type, typename reference, typename pointer>
	class iterator_impl {
		friend concurrent_vector;

	private:
		size_type idx{ 0 };
		typename root_t::data_guard vector_root;

		iterator_impl(size_type idx, 
					  typename root_t::data_guard &&vector_root) 
			: idx(idx), vector_root(std::move(vector_root)) 
		{}

	public:
		using difference_type = typename allocator_type::difference_type;
		typedef std::random_access_iterator_tag iterator_category;

		iterator_impl(iterator_impl&&) = default;
		iterator_impl& operator=(iterator_impl&&) = default;

		template <
			typename val = value_type,
			typename ref = typename allocator_type::reference,
			typename ptr = typename allocator_type::pointer,
			typename = typename std::enable_if<!std::is_same_v<ref, reference>>::type
		>
		iterator_impl(iterator_impl<val, ref, ptr> &&o)
			: idx(o.idx), vector_root(std::move(o.vector_root))
		{}

		~iterator_impl() noexcept {}

		auto location() const { return idx; }
		auto capacity() const { return vector_root->capacity; }
		auto size(std::memory_order order = std::memory_order_acquire) const {
			return std::min(vector_root->size.load(order), capacity());
		}
		bool empty_hint() const { return size(std::memory_order_relaxed) == 0; }

		bool operator==(const iterator_impl&o) const {
			return idx == o.idx;
		}
		bool operator!=(const iterator_impl&o) const {
			return !(*this == o);
		}
		bool operator<(const iterator_impl&o) const {
			return idx < o.idx;
		}
		bool operator>(const iterator_impl&o) const {
			return idx > o.idx;
		}
		bool operator<=(const iterator_impl&o) const {
			return idx <= o.idx;
		}
		bool operator>=(const iterator_impl&o) const {
			return idx >= o.idx;
		}

		iterator_impl& operator++() {
			++idx;
			return *this;
		}
		iterator_impl operator++(int) {
			auto it = *this;
			++(*this);
			return it;
		}
		iterator_impl& operator--() {
			--idx;
			return *this;
		}
		iterator_impl operator--(int) {
			auto it = *this;
			--(*this);
			return it;
		}
		iterator_impl& operator+=(size_type n) {
			idx += n;
			return *this;
		}
		iterator_impl operator+(size_type n) const { return { idx + n, std::move(vector_root) }; }
		friend iterator_impl operator+(size_type n, const iterator_impl &it) { return { it.idx + n, std::move(it.vector_root) }; }
		iterator_impl& operator-=(size_type n) {
			idx -= n;
			return *this;
		}
		iterator_impl operator-(size_type n) const { return { idx + n, std::move(vector_root) }; }
		difference_type operator-(const iterator_impl &it) const { return static_cast<difference_type>(idx - it.idx); }

		reference operator*() const { return *(vector_root->storage + idx); }
		pointer operator->() const { return vector_root->storage + idx; }
		reference operator[](size_type n) const { return *(vector_root->storage + idx + n); }
	};

public:
	using iterator = iterator_impl<element_type, element_type&, element_type*>;
	using const_iterator = iterator_impl<const element_type, const element_type&, const element_type*>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	mutable root_t root;

	storage_allocator_type storage_allocator;

private:
	auto root_guard(std::memory_order order = std::memory_order_acquire) const { return root.acquire(order); }

	auto resize(std::size_t requested_new_size, std::size_t minimal_acceptable_size = requested_new_size) {
		auto r = root_guard();

		if (r->capacity >= requested_new_size) {
			// Nothing to do
			return;
		}

		// Create resize data
		resize_data rd(requested_new_size, r, this);
		resize_data *old = nullptr;
		while (!r->resize.compare_exchange_strong(old, &rd)) {
			// A resize is already in progress. Join it
			join_resize_and_acquire(r);
			// If the new size is at least the minimal acceptable size, we are done. Otherwise resize again.
			if (r->capacity >= minimal_acceptable_size)
				return r;

			// Try to resize again
			old = nullptr;
		}

		// Resize initiated
		resize->join();

		// Return new root
		return root_guard();
	}

	void join_resize_and_acquire(typename root_t::data_guard &r) {
		if (auto resize = r->resize.load()) {
			// Joined a resize
			resize->join();

			// Done, get new root.
			r = root_guard();
		}
	}

	// A mutation is considered successful iff during the mutation the root wasn't changed by a resize.
	// Otherwise the resize operation can undo the mutation.
	bool mutate(const_iterator &i) {
		// Finish any ongoing resize operations
		auto r = join_resize_and_acquire();
		if (i.vector_root == r)
			return true;

		// Update iterator to new root
		i = it(i.idx);
		return false;
	}

public:
	concurrent_vector()
		: root(initial_count)
	{}
	template <typename... Args>
	explicit concurrent_vector(std::size_t count, Args&&... args)
		: root(count, std::forward<Args>(args)...)
	{}
	concurrent_vector(std::initializer_list<T> il)
		: root(il.size())
	{
		for (auto &t : il)
			emplace_back(std::move(t));
	}

	~concurrent_vector() noexcept {}

	element_type& operator[](int i) {
		return *it(i);
	}
	const element_type& operator[](int i) const {
		return *it(i);
	}

	iterator it(size_type i) { return iterator(i, root_guard()); }
	const_iterator it(size_type i) const { return const_iterator(i, root_guard()); }
	const_iterator cit(size_type i) const { return const_iterator(i, root_guard()); }
	reverse_iterator rit(size_type i) { return reverse_iterator(i, root_guard()); }
	const_reverse_iterator rit(size_type i) const { return const_reverse_iterator(i, root_guard()); }
	const_reverse_iterator crit(size_type i) const { return const_reverse_iterator(i, root_guard()); }

	template <typename... Ts>
	void emplace_back(Ts&&... ts) {
		std::size_t location;
		{
			// Finish any pending resizes
			auto r = root_guard();
			join_resize_and_acquire(r);

			// Atomically increase vector size
			location = r->size.fetch_add(1);
			iterator i(location, std::move(r));

			// If we have space, emplace
			if (r->capacity > location) {
				emplace(i, std::forward<Ts>(ts)...);
				return;
			}
		}

		// Otherwse, resize and emplace at choosen location.
		auto r = resize(location * 2, location + 1);
		emplace(iterator(location, std::move(r)), 
				std::forward<Ts>(ts)...);
	}

	template <typename... Ts>
	void insert(Ts&&... ts) {
		for (auto i = it(0); i.location() < i.size(); ++i) {
			auto guard = *i;

			// Find an empty slot and try to insert
			if (!guard.is_valid()) {
				bool success;
				do {
					success = i->try_compare_emplace(std::memory_order_acq_rel, guard, std::forward<Ts>(ts)...);
					// Make sure we managed to take ownership of i AND that the mutation is valid.
				} while (success && !mutate(i));

				if (success)
					return;
			}
		}

		// No empty slots found
		emplace_back(std::forward<Ts>(ts)...);
	}

	template <typename... Ts>
	void emplace(const_iterator &i, Ts&&... ts) {
		do {
			i->emplace(std::memory_order_acq_rel, std::forward<Ts>(ts)...);
		} while (!mutate(i));
	}
	void erase(const_iterator &i) {
		do {
			i->drop();
		} while (!mutate(i));
	}

	void reserve(std::size_t size) {
		resize(size);
	}
};

}
