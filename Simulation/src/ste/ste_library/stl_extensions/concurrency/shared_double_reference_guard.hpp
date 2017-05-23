// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atomic>
#include <ref_count_ptr.hpp>

namespace ste {

namespace _shared_double_reference_guard_detail {

template <typename data_t, class Allocator = std::allocator<data_t>>
class data_factory {
public:
	using allocator_type = Allocator;

private:
	allocator_type allocator;

public:
	void release(data_t *ptr) {
		ptr->~data_t();
		allocator.deallocate(ptr, 1);
	}
	template <typename ... Ts>
	data_t* claim(Ts&&... args) {
		auto ptr = allocator.allocate(1);
		::new (ptr) data_t(std::forward<Ts>(args)...);
		
		return ptr;
	}
};

template <typename DataType, typename Allocator>
class data {
public:
	std::atomic<int> internal_counter;
	DataType object;

	template <typename ... Ts>
	data(Ts&&... args) : internal_counter(0), object(std::forward<Ts>(args)...) {}
	data& operator=(data &&d) noexcept {
		internal_counter.store(0);
		object = std::move(d.object);
		return *this;
	}

	void release_ref() {
		if (internal_counter.fetch_add(1, std::memory_order_acquire) == -1) {
			destroy();
		}
	}

private:
	static data_factory<
		data<DataType, Allocator>, typename Allocator::template rebind<data<DataType, Allocator>>::other
	> factory;

public:
	static void release(data *ptr) { factory.release(ptr); }
	template <typename ... Ts>
	static data* claim(Ts&&... args) { return factory.claim(std::forward<Ts>(args)...); }
	void destroy() {
		factory.release(this);
	}
};

template <typename DataType, typename Allocator>
data_factory<
	data<DataType, Allocator>, typename Allocator::template rebind<data<DataType, Allocator>>::other
> data<DataType, Allocator>::factory;

}

template <typename DataType, typename Allocator>
class shared_double_reference_guard {
private:
	using data_t = _shared_double_reference_guard_detail::data<DataType, Allocator>;

public:
	template <typename T>
	class data_guard_t {
		friend class shared_double_reference_guard<DataType, Allocator>;

	private:
		T *ptr;

	public:
		data_guard_t() = delete;

		data_guard_t(T *ptr) noexcept : ptr(ptr) { }
		data_guard_t(const data_guard_t &d) = delete;
		data_guard_t &operator=(const data_guard_t &d) = delete;
		data_guard_t(data_guard_t &&d) noexcept {
			ptr = d.ptr;
			d.ptr = 0;
		}
		data_guard_t &operator=(data_guard_t &&d) noexcept {
			if (ptr) ptr->release_ref();
			ptr = d.ptr;
			d.ptr = 0;
			return *this;
		}

		~data_guard_t() noexcept { if (ptr) ptr->release_ref(); }

		bool operator==(const data_guard_t &rhs) const { return ptr == rhs.ptr; }
		bool operator!=(const data_guard_t &rhs) const { return ptr != rhs.ptr; }

		bool is_valid() const { return !!ptr; }

		DataType* operator->() { return &ptr->object; }
		DataType& operator*() { return ptr->object; }
		const DataType* operator->() const { return &ptr->object; }
		const DataType& operator*() const { return ptr->object; }

		operator bool() const { return is_valid(); }
	};

	using data_ptr = ref_count_ptr<data_t*>;
	using data_guard = data_guard_t<data_t>;
	using const_data_guard = data_guard_t<const data_t>;

	/**
	 *	@brief	Creates an instance of the internal reference counted pointer, with initial reference count of 2. Returns a guard to it.
	 *			Used exclusively for compare_exchange.
	 */
	template <typename... Args>
	static data_guard create_data_ptr(data_ptr &ptr, Args&&... args) {
		data_t *new_data = data_t::claim(std::forward<Args>(args)...);
		ptr = data_ptr{ 2, new_data };

		return data_guard(ptr.get());
	}

private:
	mutable std::atomic<data_ptr> guard;

	void release(data_ptr &old_data_ptr) {
		if (!old_data_ptr.get())
			return;
		auto external = old_data_ptr.get_counter();
		if (old_data_ptr.get()->internal_counter.fetch_sub(external, std::memory_order_release) == external - 1)
			old_data_ptr.get()->destroy();
		else
			old_data_ptr.get()->release_ref();
	}

public:
	shared_double_reference_guard() {
		data_ptr new_data_ptr{ 0, nullptr };
		guard.store(new_data_ptr);

		assert(guard.is_lock_free() && "guard not lock free");
	}

	template <typename ... Ts>
	shared_double_reference_guard(Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };
		guard.store(new_data_ptr);

		assert(guard.is_lock_free() && "guard not lock free");
	}

	~shared_double_reference_guard() {
		data_ptr old_data_ptr = guard.load();
		release(old_data_ptr);
	}

	/**
	*	@brief	Acquires a lock to the guarded object.
	*/
	data_guard acquire(std::memory_order order = std::memory_order_acquire) {
		data_ptr new_data_ptr;
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		do {
			new_data_ptr = old_data_ptr;
			new_data_ptr.inc();
		} while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order));

		return data_guard(new_data_ptr.get());
	}
	/**
	*	@brief	Acquires a lock to the guarded object.
	*/
	const_data_guard acquire(std::memory_order order = std::memory_order_acquire) const {
		data_ptr new_data_ptr;
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		do {
			new_data_ptr = old_data_ptr;
			new_data_ptr.inc();
		} while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order));

		return data_guard(new_data_ptr.get());
	}

	/**
	*	@brief	Emplaces a new value and acquires a lock to the guarded object.
	*/
	template <typename ... Ts>
	data_guard emplace_and_acquire(std::memory_order order, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 2, new_data };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order)) {}

		release(old_data_ptr);

		return data_guard(new_data_ptr.get());
	}

	/**
	*	@brief	Emplaces a new value.
	*/
	template <typename ... Ts>
	void emplace(std::memory_order order, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order)) {}

		release(old_data_ptr);
	}

	/**
	*	@brief	Atomically compares the current value to a previously acquired lock, and if matches emplaces new data. 
	*			Irregardless of success or failure, old_data remains a valid lock to the previous value.
	*/
	template <typename ... Ts>
	bool try_compare_emplace(std::memory_order order, const data_guard &old_data, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };

		auto success = compare_exchange(order, old_data, new_data_ptr);
		if (!success)
			data_t::release(new_data);

		return success;
	}

	/**
	*	@brief	Atomically compares the current value to a previously acquired lock, and if matches emplaces new data.
	*	
	*			Dangerous version of try_compare_emplace() which doesn't create a new object but uses a presupplied new_data_ptr created via create_data_ptr().
	*			It is the callers responsibility to ensure the correct reference count of new_data_ptr and manual release of its copies.
	*/
	bool compare_exchange(std::memory_order order, const data_guard &old_data, const data_ptr &new_data_ptr) {
		assert(new_data_ptr.get_counter() > 0);

		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);

		bool success = false;
		while (old_data_ptr.get() == old_data.ptr &&
			   !(success = guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order))) {
		}
		if (success)
			release(old_data_ptr);

		return success;
	}

	/**
	*	@brief	Checks if there is a valid value.
	*/
	bool is_valid_hint(std::memory_order order = std::memory_order_relaxed) const {
		return !!guard.load(order).get();
	}

	/**
	*	@brief	Invalidates the guard. 
	*			Does not invalidate any locks that are currently held to previous data.
	*/
	void drop() {
		data_ptr new_data_ptr{ 0, nullptr };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, std::memory_order_acq_rel)) {}

		release(old_data_ptr);
	}
};

}
