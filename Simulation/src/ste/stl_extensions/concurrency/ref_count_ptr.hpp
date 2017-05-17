// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <atomic>

namespace ste {

namespace _detail {

template <typename T, int ptr_size>
class ref_count_ptr {};
template <typename T>
class ref_count_ptr<T, 4> {
	std::uint32_t counter;
	T *ptr;

public:
	ref_count_ptr() = default;
	ref_count_ptr(std::uint32_t counter, T *ptr)
		: counter(counter), ptr(ptr)
	{}

	auto* get() const { return ptr; }
	void set(T* ptr) { this->ptr = ptr; }

	auto get_counter() const { return counter; }
	void set_counter(std::uint32_t c) { counter = c; }
	void inc() { ++counter; }
	void dec() { --counter; }
};
template <typename T>
class ref_count_ptr<T, 8> {
	// For AMD64 we use the upper 16-bit for the counter, and the lower 48-bit for the pointer.
	std::uint64_t data;

	static std::uint64_t create(std::uint32_t counter, T *ptr) {
		std::uint64_t data = static_cast<std::uint64_t>(counter) << 48;
		std::uint64_t ptr64 = reinterpret_cast<std::uint64_t>(ptr);
		return data + ptr64;
	}

public:
	ref_count_ptr() = default;
	ref_count_ptr(std::uint32_t counter, T *ptr)
		: data(create(counter, ptr))
	{}

	auto* get() const { return reinterpret_cast<T*>(data & 0xFFFF'FFFF'FFFF); }
	void set(T* ptr) {
		data = create(get_counter(), ptr);
	}

	auto get_counter() const { return static_cast<std::uint32_t>(data >> 48); }
	void set_counter(std::uint32_t c) {
		data = create(c, get());
	}
	void inc() { set_counter(get_counter() + 1); }
	void dec() { set_counter(get_counter() - 1); }
};

}

template <typename T>
using ref_count_ptr = _detail::ref_count_ptr<T, sizeof(void*)>;

}
