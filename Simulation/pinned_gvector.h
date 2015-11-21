// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "ShaderStorageBuffer.h"
#include "mapped_buffer_object_unique_ptr.h"

#include "range_lockable.h"

#include <vector>
#include <functional>
#include <type_traits>

#include <cstring>

#include <memory>

namespace StE {
namespace LLR {

template <typename T, bool lockless = false>
class pinned_gvector {
public:
	static constexpr BufferUsage::buffer_usage usage = static_cast<BufferUsage::buffer_usage>(BufferUsage::BufferUsageMapCoherent | BufferUsage::BufferUsageMapRead | BufferUsage::BufferUsageMapWrite | BufferUsage::BufferUsageMapPersistent);
	using value_type = T;

private:
	using buffer_type = ShaderStorageBuffer<T, usage>;
	using mapped_type = mapped_buffer_object_unique_ptr<T, usage>;

	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	std::size_t len;
	std::unique_ptr<buffer_type> buffer;
	mapped_type ptr;

public:
	pinned_gvector(std::size_t size = 10) : pinned_gvector(size, nullptr) {};
	pinned_gvector(const std::vector<T> &data) : pinned_gvector(data.size(), &data[0]) {};
	pinned_gvector(std::size_t size, const T *data) : len(data ? size : 0), buffer(std::make_unique<buffer_type>(size, data)) {
		ptr = buffer->map_rw(size);
	}

	void reserve(std::size_t s) {
		assert(s > len);

		auto new_buffer = std::make_unique<buffer_type>(s);
		auto new_ptr = new_buffer->map_rw(s);

		if (!lockless) {
			range<> lock_range{ 0, len * sizeof(T) };
			ptr.client_wait(lock_range);
		}

		memcpy(new_ptr.get(), ptr.get(), sizeof(T) * len);
		
		ptr = std::move(new_ptr);
		buffer = std::move(new_buffer);
	}

	void push_back(const T &t) {
		if (buffer->size() == len - 1)
			reserve(buffer->size() << 1);
		ptr.get()[len++] = t;
	}

	void push_back(const std::vector<T> &t) {
		if (buffer->size() < len + t.size())
			reserve((len + t.size()) << 1);
		memcpy(&ptr.get()[len], &t[0], sizeof(T) * t.size());
		len += t.size();
	}

	void pop_back() {
		assert(len);

		if (!lockless) {
			range<> lock_range{ (len - 1) * sizeof(T), sizeof(T) };
			ptr.client_wait(lock_range);
		}

		--len;
	}

	void insert(std::size_t n, const T &t) {
		assert(n <= len && "Subscript out of range.");

		if (n == len) {
			push_back(t);
			return;
		}

		if (buffer->size() == len - 1) {
			std::size_t s = buffer->size() << 1;

			auto new_buffer = std::make_unique<buffer_type>(s);
			auto new_ptr = new_buffer->map_rw(s);

			if (!lockless) {
				range<> lock_range{ 0, len * sizeof(T) };
				ptr.client_wait(lock_range);
			}

			memcpy(new_ptr.get(), ptr.get(), n * sizeof(T));
			new_ptr.get()[n] = t;
			memcpy(new_ptr.get() + n + 1, ptr.get() + n, (len - n) * sizeof(T));

			++len;

			ptr = std::move(new_ptr);
			buffer = std::move(new_buffer);

			return;
		}

		if (!lockless) {
			range<> lock_range{ n * sizeof(T), (len - n) * sizeof(T) };
			ptr.client_wait(lock_range);
		}
		
		memmove(&ptr.get()[n + 1], &ptr.get()[n], (len - n) * sizeof(T));
		ptr.get()[n] = t;

		++len;
	}

	void erase(std::size_t n) {
		assert(n < len && "Subscript out of range.");

		if (n == len - 1) {
			pop_back();
			return;
		}

		if (!lockless) {
			range<> lock_range{ n * sizeof(T), (len - n) * sizeof(T) };
			ptr.client_wait(lock_range);
		}

		--len;

		memmove(&ptr.get()[n], &ptr.get()[n + 1], (len - n) * sizeof(T));
	}

	template <typename ... Args>
	void emplace(std::size_t n, Args&&... args) {
		assert(n <= len && "Subscript out of range.");

		if (n == len) {
			push_back(T(std::forward<Args>(args)...));
			return;
		}

		if (!lockless) {
			range<> lock_range{ n * sizeof(T), sizeof(T) };
			ptr.client_wait(lock_range);
		}

		ptr.get()[n] = T(std::forward<Args>(args)...);
	}

	template <bool b = !lockless>
	void lock_range(const range<> &r, std::enable_if_t<b>* = 0) const { ptr.lock(r); }

	const T &operator[](std::size_t n) const {
		assert(n < len && "Subscript out of range."); 
		return ptr.get()[n];
	}
	const auto &get_buffer() const { return *buffer; }
	const auto size() const { return len; }
};

}
}
