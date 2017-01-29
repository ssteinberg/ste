// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gstack.hpp"
#include "range.hpp"

#include <vector>
#include <functional>
#include <type_traits>

namespace StE {
namespace Core {

template <typename T, std::size_t max_size = 4096>
class ring_buffer {
public:
	using value_type = T;
	using storage_type = gstack<T>;
	static constexpr auto usage = storage_type::usage;

	static_assert(max_size > 0, "max_size must be positive");
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	storage_type storage;
	std::size_t offset{ 0 };

public:
	ring_buffer() : ring_buffer(std::min<std::size_t>(8, max_size), nullptr) {}
	ring_buffer(const std::vector<T> &data) : ring_buffer(data.size(), &data[0]) { assert(data.size() < max_size); }
	ring_buffer(std::size_t size, const T *data) : storage(size, data) {
		if (storage.size() == 0)
			storage.reserve(1);
		assert(size <= max_size);
	}

	range<> commit(const T &t) {
		if (storage.size() + 1 < max_size) {
			storage.push_back(t);
			return { (storage.size() - 1) * sizeof(T), sizeof(T) };
		}

		if (offset + 1 >= max_size)
			offset = 0;

		range<> r{ offset * sizeof(T), sizeof(T) };
		storage.overwrite(offset++, t);

		return r;
	}

	range<> commit(const std::vector<T> &v) {
		assert(v.size() < max_size);

		if (storage.size() + v.size() < max_size) {
			storage.push_back(v);
			return { (storage.size() - v.size()) * sizeof(T), v.size() * sizeof(T) };
		}

		if (offset + v.size() > max_size)
			offset = 0;

		range<> r{ offset * sizeof(T), v.size() * sizeof(T) };
		storage.overwrite(offset, v);
		offset += v.size();

		return r;
	}

	const auto &get_buffer() const { return storage.get_buffer(); }
	auto size() const { return storage.size(); }
};

}
}
