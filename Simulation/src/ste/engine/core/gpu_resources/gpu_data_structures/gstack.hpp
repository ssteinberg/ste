// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "ShaderStorageBuffer.hpp"

#include <vector>
#include <functional>
#include <type_traits>

namespace StE {
namespace Core {

template <typename T, bool lockless = false>
class gstack {
public:
	static constexpr BufferUsage::buffer_usage usage = static_cast<BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageDynamic | Core::BufferUsage::BufferUsageSparse);
	using value_type = T;

private:
	static constexpr int pages = 8192;

	using buffer_type = ShaderStorageBuffer<T, usage>;

	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	std::size_t len{ 0 };
	buffer_type buffer;

public:
	gstack(std::size_t size = 10) : gstack(size, nullptr) {}
	gstack(const std::vector<T> &data) : gstack(data.size(), &data[0]) {}
	gstack(std::size_t size, const T *data) : len(data ? size : 0), buffer(pages * std::max(65536, buffer_type::page_size()) / sizeof(T)) {
		if (data)
			for (std::size_t i = 0; i < size; ++i)
				push_back(data[i]);
	}

	void reserve(std::size_t n) {
		buffer.commit_range(0, n);

		len = std::max(n, len);
	}

	void push_back(const T &t) {
		buffer.commit_range(len, 1);
		buffer.upload(len, 1, &t);

		++len;
	}

	void push_back(const std::vector<T> &t) {
		buffer.commit_range(len, t.size());
		buffer.upload(len, t.size(), &t[0]);

		len += t.size();
	}

	void pop_back(std::size_t n = 1) {
		assert(len - n >= 0 && n > 0 && "Subscript out of range.");

		if (!lockless) {
			range<> lock_range{ (len - n) * sizeof(T), n * sizeof(T) };
			buffer.client_wait_for_range(lock_range);
		}

		len -= n;
	}

	void overwrite(std::size_t n, const T &t) {
		assert(n < len && "Subscript out of range.");

		if (!lockless) {
			range<> lock_range{ (len - n) * sizeof(T), sizeof(T) };
			buffer.client_wait_for_range(lock_range);
		}

		buffer.upload(n, 1, &t);
	}

	void overwrite(std::size_t n, const std::vector<T> &t) {
		assert(n < len && "Subscript out of range.");

		if (!lockless) {
			range<> lock_range{ (len - n) * sizeof(T), t.size() * sizeof(T) };
			buffer.client_wait_for_range(lock_range);
		}

		if (n + t.size() > len) {
			buffer.commit_range(len, n + t.size() - len);
			len = n + t.size();
		}
		buffer.upload(n, t.size(), &t[0]);
	}

	void overwrite_all(const gli::format format, const void *data, const gli::swizzles &swizzle = swizzles_rgba) {
		if (!lockless) {
			range<> lock_range{ 0, len * sizeof(T) };
			buffer.client_wait_for_range(lock_range);
		}

		buffer.clear(format, data, 0, size(), swizzle);
	}
	void overwrite_all(const gli::format format, const void *data, int offset, std::size_t size, const gli::swizzles &swizzle = swizzles_rgba) {
		assert(offset + size <= len && "Out of range.");

		if (!lockless) {
			range<> lock_range{ offset, size * sizeof(T) };
			buffer.client_wait_for_range(lock_range);
		}

		buffer.clear(format, data, offset, size, swizzle);
	}

	template <bool b = !lockless>
	void lock_range(const range<> &r, std::enable_if_t<b>* = 0) const { buffer.lock_range(r); }

	const auto &get_buffer() const { return buffer; }
	const auto size() const { return len; }
};

}
}
