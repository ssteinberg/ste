// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <memory>
#include <type_traits>
#include <algorithm>

namespace ste {

template <typename T, std::size_t min_alignment = 64, bool pad_to_alignment = true>
class aligned_ptr {
public:
	using value_type = T;
	static constexpr std::size_t alignment = std::max<std::size_t>(min_alignment, alignof(T));

private:
	static constexpr std::size_t block_size = pad_to_alignment ? alignment - 1u + sizeof(value_type) : sizeof(value_type);

	template <typename... Ts>
	static auto make_ptr(void *p,
						 Ts&&... ts) {
		auto space = block_size;
		std::align(alignment, sizeof(value_type), p, space);

		auto ptr = static_cast<value_type*>(p);
		new (ptr) value_type(std::forward<Ts>(ts)...);

		return ptr;
	}

	static void deleter(value_type *ptr) {
		ptr->~value_type();
	}

private:
	std::unique_ptr<std::int8_t[]> block;
	std::unique_ptr<value_type, decltype(&deleter)> ptr;

public:
	template <typename... Ts>
	aligned_ptr(Ts&&... ts)
		: block(new std::int8_t[block_size]),
		ptr(make_ptr(block.get(), std::forward<Ts>(ts)...), deleter)
	{}
	// msvc 14.10.25017 (VC++ 2017 March release) fails with an internal error with conditional noexcept
//	~aligned_ptr() noexcept(std::is_nothrow_destructible_v<value_type>) {}
	~aligned_ptr() noexcept {}

	aligned_ptr(aligned_ptr&&) = default;
	aligned_ptr &operator=(aligned_ptr&&) = default;

	auto *get() { return ptr.get(); }
	auto *get() const { return ptr.get(); }
	auto *operator->() { return get(); }
	auto *operator->() const { return get(); }
	auto& operator*() { return *get(); }
	auto& operator*() const { return *get(); }
};

}
