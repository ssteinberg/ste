// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "range.h"

#include <memory>
#include <functional>

namespace StE {
namespace LLR {

template <typename Type, BufferUsage::buffer_usage U>
class mapped_buffer_object_unique_ptr : public std::unique_ptr<Type, std::function<void(Type*)>> {
private:
	using T = std::remove_cv_t<Type>;

	friend class buffer_object<T, U>;
	using Base = std::unique_ptr<Type, std::function<void(Type*)>>;

private:
	unsigned int buffer_id{ 0 };
	range<> mapped_range;
	buffer_object<T, U> *buffer_object_ptr{ nullptr };

public:
	mapped_buffer_object_unique_ptr() = default;

	mapped_buffer_object_unique_ptr(mapped_buffer_object_unique_ptr &&m) = default;
	mapped_buffer_object_unique_ptr& operator=(mapped_buffer_object_unique_ptr &&m) = default;
	mapped_buffer_object_unique_ptr(const mapped_buffer_object_unique_ptr &c) = delete;
	mapped_buffer_object_unique_ptr& operator=(const mapped_buffer_object_unique_ptr &c) = delete;

	void flush(int offset, int length) {
		if (buffer_id) glFlushMappedNamedBufferRange(buffer_id, offset, length);
	}

	bool is_valid() const { return !!buffer_id; }

	void lock() const { buffer_object_ptr->lock_range(mapped_range); }
	void lock(const range<> &sub_range) const {
		range<> r;
		r.start = sub_range.start + mapped_range.start;
		r.length = sub_range.length;
		buffer_object_ptr->lock_range(r);
	}
	void wait() const { buffer_object_ptr->wait_for_range(mapped_range); }
	void wait(const range<> &sub_range) const {
		range<> r;
		r.start = sub_range.start + mapped_range.start;
		r.length = sub_range.length;
		buffer_object_ptr->wait_for_range(r);
	}

protected:
	mapped_buffer_object_unique_ptr(Type *map, unsigned int id, buffer_object<T, U> *bo, const range<> &r) :
		Base(map, [this](Type* ptr) { if (buffer_id) glUnmapNamedBuffer(buffer_id); }),
		buffer_id(id),
		buffer_object_ptr(bo),
		mapped_range(r) {}

	unsigned int get_buffer_id() const { return buffer_id; }
};

}
}
