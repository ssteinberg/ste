// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <memory>
#include <atomic>

#include <shared_double_reference_guard.hpp>

#include <initializer_list>
#include <random>
#include <algorithm>

namespace ste {

/**
*	@brief	Concurrent, lock-free, dynamic vector.
*/
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
	template <typename shared_double_ref_guard_data, typename data_guard>
	struct resize_data {
		concurrent_vector *vec;

		data_guard old_data;
		std::size_t old_size;

		typename shared_double_ref_guard_data::data_ptr new_data_ptr;
		typename shared_double_ref_guard_data::data_guard new_data;
		std::size_t new_size;

		std::atomic<std::uint8_t> resize_complete{ 0 };

		resize_data(std::size_t new_size, data_guard &&old_guard, concurrent_vector *vec)
			: vec(vec), 
			old_data(std::move(old_guard)), 
			old_size(old_data->capacity), 
			new_data(shared_double_ref_guard_data::create_data_ptr(new_data_ptr, new_size)),
			new_size(new_size)
		{}

		void join() {
			if (resize_complete.load(std::memory_order_acquire) == 1)
				return;

			// Choose starting point at random
			std::random_device rd;
			std::mt19937 mt(rd());
			std::size_t start = std::uniform_int_distribution<std::size_t>(0, old_size)(mt);

			// Transfer elements to new vector
			auto new_storage = new_data->storage;
			for (std::size_t i = start; i<start + old_size; ++i) {
				auto idx = i % old_size;

				auto old_guard = (old_data->storage + idx)->acquire();
				if (old_guard.is_valid()) {
					// Try to copy to new vector, unless someone already did that
					auto new_guard = (new_storage + idx)->acquire();
					if (!new_guard.is_valid())
						(new_storage + idx)->try_compare_emplace(std::memory_order_acq_rel, new_guard, *old_guard);
				}

				// If we are already done, exit early.
				if (resize_complete.load(std::memory_order_relaxed) == 1)
					return;
			}

			// Try to replace old vector with new
			vec->root.compare_exchange(std::memory_order_acq_rel, old_data, new_data_ptr);
			resize_complete.store(1);

			old_data->resize = nullptr;
		}
	};

	struct data {
		using shared_double_ref_guard_data = shared_double_reference_guard<data, storage_allocator_type>;
		using double_ref_guard_data_ptr = typename shared_double_reference_guard<data, storage_allocator_type>::data_ptr;
		using resize_data_t = resize_data<shared_double_ref_guard_data, typename shared_double_ref_guard_data::data_guard>;
		using resize_data_ptr_t = std::shared_ptr<resize_data_t>;

		element_type* storage;
		std::size_t capacity;

		resize_data_ptr_t resize;

		storage_allocator_type storage_allocator;

		data(std::size_t count) : capacity(count) {
			storage = storage_allocator.allocate(count);
			for (std::size_t i = 0; i < count; ++i)
				::new (storage + i) element_type();
		}
		template <typename... Args>
		data(std::size_t count, Args&&... args) : capacity(count) {
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

	using root_t = typename data::shared_double_ref_guard_data;
	using resize_data_t = typename data::resize_data_t;
	using resize_data_ptr_t = typename data::resize_data_ptr_t;

	static constexpr auto initial_count = 8;

	using resize_data_allocator_type = typename Allocator::template rebind<resize_data_t>::other;

private:
	template <typename val, typename ref, typename ptr>
	class iterator_impl {
		friend concurrent_vector;
		using reference = ref;
		using pointer = ptr;

	protected:
		size_type idx{ 0 };
		val vector;

		iterator_impl(size_type idx, 
					  val vector)
			: idx(idx), vector(vector) 
		{}

	public:
		using difference_type = typename allocator_type::difference_type;
		typedef std::random_access_iterator_tag iterator_category;

		iterator_impl(iterator_impl&&) = default;
		iterator_impl& operator=(iterator_impl&&) = default;

		virtual ~iterator_impl() noexcept {}

		auto location() const { return idx; }

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
		iterator_impl operator+(size_type n) const { return { idx + n, vector }; }
		friend iterator_impl operator+(size_type n, const iterator_impl &it) { return { it.idx + n, it.vector }; }
		iterator_impl& operator-=(size_type n) {
			idx -= n;
			return *this;
		}
		iterator_impl operator-(size_type n) const { return { idx + n, vector }; }
		difference_type operator-(const iterator_impl &it) const { return static_cast<difference_type>(idx - it.idx); }

		reference operator*() const {
			auto root_guard = vector->root.acquire();
			return *(root_guard->storage + idx);
		}
		pointer operator->() const {
			auto root_guard = vector->root.acquire(); 
			return root_guard->storage + idx;
		}
		reference operator[](size_type n) const {
			auto root_guard = vector->root.acquire(); 
			return *(root_guard->storage + idx + n);
		}
	};

public:
	class const_iterator : public iterator_impl<const concurrent_vector*, const element_type&, const element_type*> {
		using Base = iterator_impl<const concurrent_vector*, const element_type&, const element_type*>;
		friend concurrent_vector;
	public:
		using Base::Base;
	};
	class iterator : public iterator_impl<concurrent_vector*, element_type&, element_type*> {
		using Base = iterator_impl<concurrent_vector*, element_type&, element_type*>;
		friend concurrent_vector;
	public:
		using Base::Base;
		iterator(const_iterator &&i) noexcept : Base(i.idx, i.vector) {}
	};
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using reverse_iterator = std::reverse_iterator<iterator>;

private:
	mutable root_t root;
	std::atomic<std::size_t> elements{ 0 };

	storage_allocator_type storage_allocator;
	resize_data_allocator_type resize_data_allocator;

private:
	auto root_guard(std::memory_order order = std::memory_order_acquire) const { return root.acquire(order); }

	auto resize(std::size_t requested_new_size, std::size_t minimal_acceptable_size = requested_new_size) {
		auto r = root_guard();

		if (r->capacity >= requested_new_size) {
			// Nothing to do
			return r;
		}

		// Create resize data
		auto rd = std::allocate_shared<resize_data_t, resize_data_allocator_type>(resize_data_allocator,
																				  requested_new_size, 
																				  root_guard(), 
																				  this);

		auto old = resize_data_ptr_t(nullptr);
		while (!std::atomic_compare_exchange_strong(&r->resize, &old, rd)) {
			// A resize is already in progress. Join in
			join_resize_and_acquire(r);
			// If the new size is at least the minimal acceptable size, we are done. Otherwise resize again.
			if (r->capacity >= minimal_acceptable_size)
				return r;

			// Try to resize again
			old = resize_data_ptr_t(nullptr);
		}

		// Resize initiated
		rd->join();
		rd = nullptr;

		// Return new root
		return root_guard();
	}

	void join_resize_and_acquire(typename root_t::data_guard &r) {
		if (resize_data_ptr_t resize = std::atomic_load(&r->resize)) {
			// Joined a resize
			resize->join();
			resize = nullptr;

			// Done, get new root.
			r = root_guard();
		}
	}

	// A mutation is considered successful iff during the mutation the root wasn't changed by a resize.
	// Otherwise the resize operation can undo the mutation.
	bool mutate(typename root_t::data_guard &mutated_vector) {
		auto r = root_guard();

		// Finish any ongoing resize operations
		join_resize_and_acquire(r);
		if (mutated_vector == r)
			return true;

		// Update iterator to new root
		mutated_vector = std::move(r);
		return false;
	}

	template <typename... Ts>
	void _emplace_internal(iterator &iter, typename root_t::data_guard &r, Ts&&... ts) {
		do {
			assert(size() > iter.idx);
			iter->emplace(std::memory_order_acq_rel, std::forward<Ts>(ts)...);
		} while (!mutate(r));
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

	/**
	*	@brief	Returns a reference to position 'i'.
	*/
	element_type& operator[](int i) {
		return *it(i);
	}
	/**
	*	@brief	Returns a const reference to position 'i'.
	*/
	const element_type& operator[](int i) const {
		return *it(i);
	}

	iterator it(size_type i) { return iterator(i, this); }
	const_iterator it(size_type i) const { return const_iterator(i, this); }
	const_iterator cit(size_type i) const { return const_iterator(i, this); }
	reverse_iterator rit(size_type i) { return reverse_iterator(i, this); }
	const_reverse_iterator rit(size_type i) const { return const_reverse_iterator(i, this); }
	const_reverse_iterator crit(size_type i) const { return const_reverse_iterator(i, this); }

	iterator begin() { return iterator(0, this); }
	const_iterator begin() const { return const_iterator(0, this); }
	const_iterator cbegin() const { return const_iterator(0, this); }
	iterator end() { return iterator(size(), this); }
	const_iterator end() const { return const_iterator(size(), this); }
	const_iterator cend() const { return const_iterator(size(), this); }
	reverse_iterator rbegin() { return reverse_iterator(0, this); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(0, this); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(0, this); }
	reverse_iterator rend() { return reverse_iterator(size(), this); }
	const_reverse_iterator rend() const { return const_reverse_iterator(size(), this); }
	const_reverse_iterator crend() const { return const_reverse_iterator(size(), this); }

	/**
	*	@brief	Returns a hint suggesting whether or not the vector was empty at the time of the call.
	*/
	bool empty_hint() const { return elements.load(std::memory_order_relaxed) == 0; }
	/**
	*	@brief	Returns the current vector size.
	*/
	auto size() const {
		auto r = root_guard();
		return std::min(elements.load(), r->capacity);
	}

	/**
	*	@brief	Returns the current vector capacity.
	*/
	auto capacity() const {
		auto r = root_guard();
		return r->capacity;
	}

	/**
	*	@brief	Emplaces new value at the end of the vector.
	*			Will resize if no space left.
	*/
	template <typename... Ts>
	void emplace_back(Ts&&... ts) {
		std::size_t location;
		{
			auto r = root_guard();

			// Atomically increase vector size
			location = elements.fetch_add(1);

			// If we have space, emplace
			if (r->capacity > location) {
				_emplace_internal(it(location), r, std::forward<Ts>(ts)...);
				return;
			}
		}

		// Otherwse, resize and emplace at choosen location.
		auto required_size = location + 1;
		auto r = resize(required_size * 2, required_size);
		_emplace_internal(it(location), r, std::forward<Ts>(ts)...);
	}

	/**
	*	@brief	Emplaces new value at an empty slot.
	*			Will resize if no space left.
	*/
	template <typename... Ts>
	void insert(Ts&&... ts) {
		auto r = root_guard();
		auto iter = it(0);
		auto l = size();

		// Find an empty slot and try to insert
		while (iter.location() < l) {
			for (;iter.location() < l; ++iter) {
				auto guard = iter->acquire();

				bool success = false;
				if (!guard.is_valid()) {
					do {
						success = iter->try_compare_emplace(std::memory_order_acq_rel, guard, std::forward<Ts>(ts)...);
						// Make sure we managed to take ownership of i AND that the mutation is valid.
					} while (success && !mutate(r));

					if (success)
						return;
				}
			}
			l = size();
		}

		// No empty slots found
		emplace_back(std::forward<Ts>(ts)...);
	}

	/**
	*	@brief	Emplaces new value at the provided iterator.
	*/
	template <typename... Ts>
	void emplace(iterator &iter, Ts&&... ts) {
		auto r = root_guard();
		_emplace_internal(iter, r, std::forward<Ts>(ts)...);
	}
	/**
	*	@brief	Emplaces new value at the provided iterator, only if the iterator is invalid. 
	*			Returns true if successful, false otherwise.
	*/
	template <typename... Ts>
	bool try_emplace_if_empty(iterator &iter, Ts&&... ts) {
		auto r = root_guard();
		auto guard = iter->acquire();

		bool success = false;
		if (!guard.is_valid()) {
			do {
				success = iter->try_compare_emplace(std::memory_order_acq_rel, guard, std::forward<Ts>(ts)...);
				// Make sure we managed to take ownership of i AND that the mutation is valid.
			} while (success && !mutate(r));
		}

		return success;
	}
	/**
	*	@brief	Erases the value at the provided iterator.
	*/
	void erase(iterator &iter) {
		auto r = root_guard();
		do {
			assert(size() > iter.idx);
			iter->drop();
		} while (!mutate(r));
	}

	/**
	*	@brief	Resizes the vector to capacity of at least 'size'.
	*/
	void reserve(std::size_t size) {
		resize(size);
	}
};

}
