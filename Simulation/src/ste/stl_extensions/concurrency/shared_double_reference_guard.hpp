// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atomic>
#include <iostream>

#include <ref_count_ptr.hpp>
#include <concurrent_pointer_recycler.hpp>

namespace ste {

namespace _shared_double_reference_guard_detail {

template <typename data_t, bool b>
class data_factory;
template <typename data_t>
class data_factory<data_t, false> {
public:
	static void release(data_t *ptr) { delete ptr; }
	template <typename ... Ts>
	data_t* claim(Ts&&... args) { return new data_t(std::forward<Ts>(args)...); }
};
template <typename data_t>
class data_factory<data_t, true> : public concurrent_pointer_recycler<data_t> {};

template <typename DataType, bool recycle_pointers>
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
	static data_factory<data<DataType, recycle_pointers>, std::is_trivially_copyable<DataType>::value && recycle_pointers> recycler;

public:
	static void release(data *ptr) { recycler.release(ptr); }
	template <typename ... Ts>
	static data* claim(Ts&&... args) { return recycler.claim(std::forward<Ts>(args)...); }
	void destroy() {
		recycler.release(this);
	}
};

template <typename DataType, bool recycle_pointers>
data_factory<data<DataType, recycle_pointers>, std::is_trivially_copyable<DataType>::value && recycle_pointers> data<DataType, recycle_pointers>::recycler;

}

template <typename DataType, bool recycle_pointers>
class shared_double_reference_guard {
private:
	using data_t = _shared_double_reference_guard_detail::data<DataType, recycle_pointers>;

	using data_ptr = ref_count_ptr<data_t>;

public:
	class data_guard {
		friend class shared_double_reference_guard<DataType, recycle_pointers>;

	private:
		data_t *ptr;

	public:
		data_guard(data_t *ptr) : ptr(ptr) { }
		data_guard(const data_guard &d) = delete;
		data_guard &operator=(const data_guard &d) = delete;
		data_guard(data_guard &&d) noexcept {
			ptr = d.ptr;
			d.ptr = 0;
		}
		data_guard &operator=(data_guard &&d) noexcept {
			if (ptr) ptr->release_ref();
			ptr = d.ptr;
			d.ptr = 0;
			return *this;
		}

		~data_guard() { if (ptr) ptr->release_ref(); }

		bool is_valid() const { return !!ptr; }

		DataType* operator->() { return &ptr->object; }
		DataType& operator*() { return ptr->object; }
		const DataType* operator->() const { return &ptr->object; }
		const DataType& operator*() const { return ptr->object; }
	};

private:
	std::atomic<data_ptr> guard;

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

	data_guard acquire(std::memory_order order = std::memory_order_acquire) {
		data_ptr new_data_ptr;
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		do {
			new_data_ptr = old_data_ptr;
			new_data_ptr.inc();
		} while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order, std::memory_order_relaxed));

		return data_guard(new_data_ptr.get());
	}

	template <typename ... Ts>
	data_guard emplace_and_acquire(std::memory_order order, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 2, new_data };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order, std::memory_order_relaxed)) {}

		release(old_data_ptr);

		return data_guard(new_data_ptr.get());
	}

	template <typename ... Ts>
	void emplace(std::memory_order order, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order, std::memory_order_relaxed)) {}

		release(old_data_ptr);
	}

	template <typename ... Ts>
	bool try_emplace(std::memory_order order1, std::memory_order order2, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };
		data_ptr old_data_ptr = guard.load(order2);
		if (guard.compare_exchange_strong(old_data_ptr, new_data_ptr, order1, std::memory_order_relaxed)) {
			release(old_data_ptr);
			return true;
		}
		data_t::release(new_data);
		return false;
	}

	template <typename ... Ts>
	bool try_compare_emplace(std::memory_order order, data_guard &old_data, Ts&&... args) {
		data_t *new_data = data_t::claim(std::forward<Ts>(args)...);
		data_ptr new_data_ptr{ 1, new_data };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);

		bool success = false;
		while (old_data_ptr.get() == old_data.ptr &&
			   !(success = guard.compare_exchange_weak(old_data_ptr, new_data_ptr, order, std::memory_order_relaxed))) {}
		if (success)
			release(old_data_ptr);
		else
			delete new_data;

		return success;
	}

	bool is_valid_hint(std::memory_order order = std::memory_order_relaxed) const {
		return !!guard.load(order).get();
	}

	void drop() {
		data_ptr new_data_ptr{ 0, nullptr };
		data_ptr old_data_ptr = guard.load(std::memory_order_relaxed);
		while (!guard.compare_exchange_weak(old_data_ptr, new_data_ptr, std::memory_order_acq_rel, std::memory_order_relaxed)) {}

		release(old_data_ptr);
	}
};

}
