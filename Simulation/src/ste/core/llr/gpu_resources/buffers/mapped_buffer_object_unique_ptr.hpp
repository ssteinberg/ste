// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "range.hpp"

#include <functional>

namespace StE {
namespace LLR {

template <typename T, BufferUsage::buffer_usage U>
class buffer_object;

template <typename Type, BufferUsage::buffer_usage U>
class mapped_buffer_object_unique_ptr {
private:
	using T = std::remove_cv_t<Type>;

	friend class buffer_object<T, U>;

private:
	struct mapped_buffer_data {
		T *ptr;
		range<> mapped_range;
		buffer_object<T, U> *buffer_object_ptr{ nullptr };
	};

private:
	std::shared_ptr<mapped_buffer_data> data{ nullptr };

public:
	mapped_buffer_object_unique_ptr() = default;

	mapped_buffer_object_unique_ptr(mapped_buffer_object_unique_ptr &&m) = default;
	mapped_buffer_object_unique_ptr& operator=(mapped_buffer_object_unique_ptr &&m) = default;
	mapped_buffer_object_unique_ptr(const mapped_buffer_object_unique_ptr &) = default;
	mapped_buffer_object_unique_ptr& operator=(const mapped_buffer_object_unique_ptr &) = default;

	~mapped_buffer_object_unique_ptr() { invalidate(); }

	void flush(int offset, int length) {
		if (data->buffer_object_ptr)
			glFlushMappedNamedBufferRange(data->buffer_object_ptr->get_resource_id(), offset, length);
	}

	bool is_valid() const { return data != nullptr && data->ptr; }

	void lock() const { data->buffer_object_ptr->lock_range(data->mapped_range); }
	void lock(const range<> &sub_range) const {
		assert(sub_range.start + sub_range.length <= data->mapped_range.length && "Out of bounds");
		
		range<> r;
		r.start = sub_range.start + data->mapped_range.start;
		r.length = sub_range.length;
		data->buffer_object_ptr->lock_range(r);
	}
	void wait() const { data->buffer_object_ptr->wait_for_range(data->mapped_range); }
	void wait(const range<> &sub_range) const {
		assert(sub_range.start + sub_range.length <= data->mapped_range.length && "Out of bounds");
		
		range<> r;
		r.start = sub_range.start + data->mapped_range.start;
		r.length = data->mapped_range.length;
		data->buffer_object_ptr->wait_for_range(r);
	}
	void client_wait() const { data->buffer_object_ptr->client_wait_for_range(data->mapped_range); }
	void client_wait(const range<> &sub_range) const {
		assert(sub_range.start + sub_range.length <= data->mapped_range.length && "Out of bounds");
		
		range<> r;
		r.start = sub_range.start + data->mapped_range.start;
		r.length = data->mapped_range.length;
		data->buffer_object_ptr->client_wait_for_range(r);
	}

	void invalidate() {
		if (is_valid())
			glUnmapNamedBuffer(data->buffer_object_ptr->get_resource_id());
		if (data != nullptr)
			data->ptr = nullptr;
		data = nullptr;
	}

	auto get() { return data->ptr; }
	auto get() const { return data->ptr; }

protected:
	mapped_buffer_object_unique_ptr(Type *map, buffer_object<T, U> *bo, const range<> &r) :
		data(new mapped_buffer_data{map, r, bo}) {}
};

}
}

#include "buffer_object.hpp"
