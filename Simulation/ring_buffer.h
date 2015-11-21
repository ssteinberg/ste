// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "gstack.h"
#include "range.h"

#include <vector>

namespace StE {
namespace LLR {

template <typename T, int max_size = 4096>
class ring_buffer {
public:
	static constexpr BufferUsage::buffer_usage usage = gstack<T, true>::usage;
	using value_type = T;

	static_assert(max_size > 0, "max_size must be positive");
	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	gstack<T,true> stack;
	std::size_t offset{ 0 };

public:
	ring_buffer() : ring_buffer(std::min(8, max_size), nullptr) {}
	ring_buffer(const std::vector<T> &data) : ring_buffer(data.size(), &data[0]) { assert(data.size() < max_size); }
	ring_buffer(std::size_t size, const T *data) : stack(size, data) {
		if (stack.size() == 0)
			stack.reserve(1);
		assert(size < max_size);
	}

	range<> commit(const T &t) {
		if (stack.size() <= offset + 1)
			offset = 0;

		range<> r{ offset * sizeof(T), sizeof(T) };
		if (stack.size() + 1 >= max_size) {
			stack.get_buffer().client_wait_for_range(r);
			stack.overwrite(offset++, t);
			return r;
		}

		if (!stack.get_buffer().client_check_range(r)) {
			offset = 0;
			stack.push_back(t);
			return { (stack.size() - 1) * sizeof(T), sizeof(T) };
		}

		stack.overwrite(offset++, t);
		return r;
	}

	range<> commit(const std::vector<T> &v) {
		assert(v.size() < max_size);

		if (stack.size() <= offset + v.size())
			offset = 0;

		range<> r{ offset * sizeof(T), v.size() * sizeof(T) };
		if (stack.size() + v.size() >= max_size) {
			stack.get_buffer().client_wait_for_range(r);
			stack.overwrite(offset, v);
			offset += v.size();
			return r;
		}

		if (stack.size() < v.size() || !stack.get_buffer().client_check_range(r)) {
			offset = 0;
			stack.push_back(v);
			return { (stack.size() - v.size()) * sizeof(T), v.size() * sizeof(T) };
		}

		stack.overwrite(offset, v);
		offset += v.size();
		return r;
	}

	void lock_range(const range<> &r) const { stack.get_buffer().lock_range(r); }

	const auto &get_buffer() const { return stack.get_buffer(); }
	const auto size() const { return stack.size(); }
};

}
}
