// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <type_traits>
#include <memory>
#include <cassert>
#include <vector>

namespace ste {

template <typename T>
class static_vector {
private:
	struct default_alignment { constexpr static int align = 8; };
	struct T_alignment { constexpr static int align = alignof(T); };
	using align = typename std::conditional<
		default_alignment::align < T_alignment::align,
		T_alignment,
		default_alignment
	>::type;

	using storage_t = typename std::aligned_storage<sizeof(T), align::align>::type;

private:
	std::size_t count;
	std::unique_ptr<storage_t[]> storage;
	std::vector<bool> allocated;

public:
	static_vector(std::size_t size)
		: count(size), storage(new storage_t[size])
	{
		allocated.resize(size, false);
	}
	template <typename... Ts>
	static_vector(std::size_t size, Ts&&... ts)
		: static_vector(size)
	{
		for (std::size_t i = 0; i < count; ++i)
			emplace(i, std::forward<Ts>(ts)...);
	}
	~static_vector() noexcept(std::is_nothrow_destructible_v<T>) {
		for (std::size_t i = 0; i < count; ++i) {
			if (allocated[i])
				destroy(i);
		}
	}

	static_vector(static_vector &&) = default;
	static_vector &operator=(static_vector &&) = default;

	template <typename... Ts>
	void emplace(std::size_t pos, Ts&&... ts) {
		assert(pos < count);

		if (allocated[pos])
			destroy(pos);

		::new (storage.get() + pos) T(std::forward<Ts>(ts)...);
		allocated[pos] = true;
	}
	void destroy(std::size_t pos) {
		assert(pos < count && allocated[pos]);

		reinterpret_cast<T*>(storage.get() + pos)->~T();
		allocated[pos] = false;
	}

	auto& operator[](int i) { return *reinterpret_cast<T*>(storage.get() + i); }
	auto& operator[](int i) const { return *reinterpret_cast<T*>(storage.get() + i); }
	auto* data() { return reinterpret_cast<T*>(storage.get()); }
	auto* data() const { return reinterpret_cast<T*>(storage.get()); }
};

}
