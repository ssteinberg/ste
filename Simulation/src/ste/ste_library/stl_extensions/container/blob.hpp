// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <string>
#include <vector>
#include <array>

namespace ste {

template <typename CharAllocator = std::allocator<char>>
class blob {
public:
	using allocator_type = CharAllocator;

private:
	using storage_type = std::basic_string<char, std::char_traits<char>, allocator_type>;

private:
	storage_type storage;

public:
	blob() = default;

	blob(const std::string &str) : storage(str) {}
	blob(std::string &&str) : storage(std::move(str)) {}

	blob(const void *data, std::size_t bytes) : storage(reinterpret_cast<const char*>(data),
														bytes) {}

	template <typename T, typename A>
	blob(const std::vector<T, A> &v) : storage(reinterpret_cast<const char*>(v.data()),
											   static_cast<std::size_t>(v.size() * sizeof(T))) {}

	template <typename T, int N>
	blob(const std::array<T, N> &v) : storage(reinterpret_cast<const char*>(v.data()),
											  static_cast<std::size_t>(v.size() * sizeof(T))) {}

	template <typename T, int N>
	blob(const T v[N]) : storage(reinterpret_cast<const char*>(v),
								 static_cast<std::size_t>(N * sizeof(T))) {}

	void* data() { return storage.data(); }
	const void* data() const { return storage.data(); }
	auto size() const { return storage.size(); }
};

}
