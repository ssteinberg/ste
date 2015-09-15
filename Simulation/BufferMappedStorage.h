// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "buffer_usage.h"

namespace StE {
namespace LLR {

template <typename T, BufferUsage::buffer_usage U>
class buffer_object;

template <typename T, BufferUsage::buffer_usage U>
class BufferMappedStorage {
private:
	friend class buffer_object<T,U>;

public:
	static constexpr BufferUsage::buffer_usage access_usage = U;

private:
	unsigned int buffer_id;
	T *ptr;

public:
	BufferMappedStorage(BufferMappedStorage &&m) = default;
	BufferMappedStorage(const BufferMappedStorage &c) = delete;
	BufferMappedStorage& operator=(BufferMappedStorage &&m) = default;
	BufferMappedStorage& operator=(const BufferMappedStorage &c) = delete;

	virtual ~BufferMappedStorage() {
		if (buffer_id)
			glUnmapNamedBuffer(buffer_id);
		invalidate();
	}

	T *data() { return ptr; }
	const T *data() const { return ptr; }

	void flush(int offset, int length) {
		if (buffer_id) glFlushMappedNamedBufferRange(buffer_id, offset, length);
	}

	bool is_valid() const { return !!buffer_id; }

protected:
	BufferMappedStorage(T *map, unsigned int id) : ptr(map), buffer_id(id) {
		if (!id || !ptr) invalidate();
	}

	unsigned int get_buffer_id() const { return buffer_id; }

	void invalidate() {
		ptr = nullptr;
		buffer_id = 0;
	}
};

}
}
