// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gstack.hpp"
#include "pinned_gvector.hpp"
#include "range.hpp"

#include <vector>

namespace StE {
namespace LLR {

template <typename T, int max_size = 4096, class Storage = gstack<T, true>>
class ring_buffer {
public:
	static constexpr BufferUsage::buffer_usage usage = Storage::usage;
	using value_type = T;

	static_assert(max_size > 0, "max_size must be positive");
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	Storage storage;
	std::size_t offset{ 0 };

public:
	ring_buffer() : ring_buffer(std::min(8, max_size), nullptr) {}
	ring_buffer(const std::vector<T> &data) : ring_buffer(data.size(), &data[0]) { assert(data.size() < max_size); }
	ring_buffer(std::size_t size, const T *data) : storage(size, data) {
		if (storage.size() == 0)
			storage.reserve(1);
		assert(size < max_size);
	}

	range<> commit(const T &t) {
		if (storage.size() <= offset + 1)
			offset = 0;

		range<> r{ offset * sizeof(T), sizeof(T) };
		if (storage.size() + 1 >= max_size) {
			storage.get_buffer().client_wait_for_range(r);
			storage.overwrite(offset++, t);
			return r;
		}

		if (!storage.get_buffer().client_check_range(r)) {
			offset = 0;
			storage.push_back(t);
			return { (storage.size() - 1) * sizeof(T), sizeof(T) };
		}

		storage.overwrite(offset++, t);
		return r;
	}

	range<> commit(const std::vector<T> &v) {
		assert(v.size() < max_size);

		if (storage.size() <= offset + v.size())
			offset = 0;

		range<> r{ offset * sizeof(T), v.size() * sizeof(T) };
		if (storage.size() + v.size() >= max_size) {
			storage.get_buffer().client_wait_for_range(r);
			storage.overwrite(offset, v);
			offset += v.size();
			return r;
		}

		if (storage.size() < v.size() || !storage.get_buffer().client_check_range(r)) {
			offset = 0;
			storage.push_back(v);
			return { (storage.size() - v.size()) * sizeof(T), v.size() * sizeof(T) };
		}

		storage.overwrite(offset, v);
		offset += v.size();
		return r;
	}

	void lock_range(const range<> &r) const { storage.get_buffer().lock_range(r); }

	const auto &get_buffer() const { return storage.get_buffer(); }
	const auto size() const { return storage.size(); }
};

}
}
