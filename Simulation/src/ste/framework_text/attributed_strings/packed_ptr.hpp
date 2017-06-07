// StE
// � Shlomi Steinberg, 2015-2017

#pragma once

#include <cstddef>

namespace StE {

namespace _detail {

template <typename Ptr, int ptr_size>
class packed_ptr {};

// 32-bit
template <typename Ptr>
class packed_ptr<Ptr, 4> {
private:
	std::uint32_t integer;
	Ptr ptr;

public:
	packed_ptr() = default;
	packed_ptr(std::uint16_t integer, Ptr ptr)
		: integer(static_cast<std::uint32_t>(integer)), ptr(ptr)
	{}

	auto* get() const { return ptr; }
	void set(Ptr ptr) { this->ptr = ptr; }

	auto get_integer() const { return static_cast<std::uint16_t>(integer); }
	void set_integer(std::uint16_t c) { integer = static_cast<std::uint32_t>(c); }

	auto* operator->() { return get(); }
	const auto* operator->() const { return get(); }
	auto& operator*() { return *get(); }
	const auto& operator*() const { return *get(); }
};

// 64-bit
template <typename Ptr>
class packed_ptr<Ptr, 8> {
private:
	// For AMD64 we use the upper 16-bit for the integer, and the lower 48-bit for the pointer.
	std::uint64_t data;

	static std::uint64_t create(std::uint16_t integer, Ptr ptr) {
		std::uint64_t data = static_cast<std::uint64_t>(integer) << 48;
		std::uint64_t ptr64 = reinterpret_cast<std::uint64_t>(ptr);
		return data + ptr64;
	}

public:
	packed_ptr() = default;
	packed_ptr(std::uint16_t integer, Ptr ptr)
		: data(create(integer, ptr))
	{}

	auto* get() const { return reinterpret_cast<Ptr>(data & 0xFFFF'FFFF'FFFF); }
	void set(Ptr ptr) {
		data = create(get_integer(), ptr);
	}

	auto get_integer() const { return static_cast<std::uint16_t>(data >> 48); }
	void set_integer(std::uint16_t c) {
		data = create(c, get());
	}

	auto* operator->() { return get(); }
	const auto* operator->() const { return get(); }
	auto& operator*() { return *get(); }
	const auto& operator*() const { return *get(); }
};

}

template <typename Ptr>
using packed_ptr = _detail::packed_ptr<Ptr, sizeof(void*)>;

}
