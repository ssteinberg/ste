// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <memory>
#include <functional>

namespace StE {
namespace LLR {

template <typename T>
class BufferMappedStorage : public std::unique_ptr<T, std::function<void(T*)>> {
private:
	template <typename T, BufferUsage::buffer_usage U>
	friend class buffer_object;

	using Base = std::unique_ptr<T, std::function<void(T*)>>;

private:
	unsigned int buffer_id;

public:
	BufferMappedStorage(BufferMappedStorage &&m) = default;
	BufferMappedStorage(const BufferMappedStorage &c) = delete;
	BufferMappedStorage& operator=(BufferMappedStorage &&m) = default;
	BufferMappedStorage& operator=(const BufferMappedStorage &c) = delete;

	void flush(int offset, int length) {
		if (buffer_id) glFlushMappedNamedBufferRange(buffer_id, offset, length);
	}

	bool is_valid() const { return !!buffer_id; }

protected:
	BufferMappedStorage(T *map, unsigned int id) : 
		Base(map, [this](T* ptr) { if (buffer_id) glUnmapNamedBuffer(buffer_id); }),
		buffer_id(id) {}

	unsigned int get_buffer_id() const { return buffer_id; }
};

}
}
