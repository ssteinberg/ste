// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "bindable_resource.h"
#include "gl_utils.h"

#include "buffer_usage.h"
#include "mapped_buffer_object_unique_ptr.h"
#include "buffer_lock.h"
#include "buffer_object_allocator.h"

#include "shader_layout_bindable_resource.h"
#include "layout_binding.h"

#include "range.h"

#include <memory>
#include <list>
#include <map>

#include <type_traits>

namespace StE {
namespace LLR {

class BufferObjectBinder {
public:
	static void bind(unsigned int id, GLenum target) {
		glBindBuffer(target, id);
	}
	static void unbind(GLenum target = 0) {
		glBindBuffer(target, 0);
	}
};

#define ALLOW_BUFFER_OBJECT_CASTS	template <typename BufferTypeTo, typename BufferTypeFrom> friend BufferTypeTo buffer_object_cast(BufferTypeFrom &&s)

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class buffer_object : public bindable_resource<buffer_object_immutable_storage_allocator<Type, U>, BufferObjectBinder, GLenum> {
private:
	using Base = bindable_resource<buffer_object_immutable_storage_allocator<Type, U>, BufferObjectBinder, GLenum>;

public:
	static constexpr BufferUsage::buffer_usage access_usage = U;
	using T = Type;

	static constexpr bool dynamic_buffer = !!(access_usage & BufferUsage::BufferUsageDynamic);
	static constexpr bool map_read_allowed = !!(access_usage & BufferUsage::BufferUsageMapRead);
	static constexpr bool map_write_allowed = !!(access_usage & BufferUsage::BufferUsageMapWrite);
	static constexpr bool map_rw_allowed = map_read_allowed && map_write_allowed;

private:
	template <typename T2, BufferUsage::buffer_usage U2>
	friend class buffer_object;

	ALLOW_BUFFER_OBJECT_CASTS;

	using lock_map_type = std::multimap<range<>, buffer_lock>;

protected:
	std::size_t buffer_size;
	mutable std::shared_ptr<lock_map_type> locks{ new lock_map_type };

	using Base::bind;
	using Base::unbind;

	template <typename T2>
	buffer_object(const buffer_object<T2, U> &t) : Base(t), locks(t.locks) {
		buffer_size = sizeof(T2) * t.buffer_size / sizeof(T);
	}
	buffer_object(const buffer_object<Type, U> &t) : Base(t), locks(t.locks), buffer_size(t.buffer_size) {}

public:
	buffer_object(buffer_object &&t) = default;
	buffer_object &operator=(buffer_object &&t) = default;

	buffer_object(std::size_t size) : buffer_object(size, nullptr) {};
	buffer_object(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {};
	buffer_object(std::size_t size, const T *data) : buffer_size(size) {
		allocator.allocate_storage(get_resource_id(), buffer_size, data);
	}

	virtual ~buffer_object() noexcept {}

	void lock_range(const range<> &r) const {
		buffer_lock l;
		l.lock();
		locks->insert(std::make_pair(r, std::move(l)));
	}
	void wait_for_range(const range<> &r) const {
		for (auto it = locks->begin(); it != locks->end();) {
			if (it->first.start > r.start + r.length)
				break;
			if (it->first.overlaps(r)) {
				it->second.wait();
				it = locks->erase(it);
			}
			else
				++it;
		}
	}

	void clear(const gli::format format, const void *data) {
		auto glf = gl_utils::translate_format(format);
		glClearNamedBufferData(get_resource_id(), glf.Internal, glf.External, glf.Type, data);
	}
	void clear(const gli::format format, const void *data, int offset, std::size_t size) {
		auto glf = gl_utils::translate_format(format);
		glClearNamedBufferSubData(get_resource_id(), glf.Internal, offset * sizeof(T), size * sizeof(T), glf.External, glf.Type, data);
	}
	void invalidate_data() { glInvalidateBufferData(get_resource_id()); }
	void invalidate_data(int offset, std::size_t length) { glInvalidateBufferSubData(get_resource_id(), offset * sizeof(T), length * sizeof(T)); }

	template <typename S, BufferUsage::buffer_usage U>
	void copy_to(buffer_object<S, U> &bo) const { glCopyNamedBufferSubData(get_resource_id(), bo.get_resource_id(), 0, 0, buffer_size * sizeof(T)); }
	template <typename S, BufferUsage::buffer_usage U>
	void copy_to(buffer_object<S, U> &bo, int read_offset, int write_offset, std::size_t size) const {
		glCopyNamedBufferSubData(get_resource_id(), bo.get_resource_id(), read_offset * sizeof(T), write_offset * sizeof(S), size * sizeof(T));
	}

	template <bool b = dynamic_buffer>
	void upload(const T *data, std::enable_if_t<b>* = 0) {
		glNamedBufferSubData(get_resource_id(), 0, buffer_size * sizeof(T), data);
	}
	template <bool b = dynamic_buffer>
	void upload(int offset, std::size_t size, const T *data, std::enable_if_t<b>* = 0) {
		glNamedBufferSubData(get_resource_id(), offset * sizeof(T), size * sizeof(T), data);
	}

	template <bool b = map_read_allowed> 
	typename std::enable_if<b, mapped_buffer_object_unique_ptr<const T, U>>::type 
		map_read(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return mapped_buffer_object_unique_ptr<const T, U>(reinterpret_cast<const T*>(glMapNamedBufferRange(get_resource_id(), offset * sizeof(T), len * sizeof(T), GL_MAP_READ_BIT | flags)),
														   get_resource_id(),
														   this, 
														   { static_cast<std::size_t>(offset), len });
	}
	template <bool b = map_write_allowed> 
	typename std::enable_if<b, mapped_buffer_object_unique_ptr<T, U>>::type
		map_write(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return mapped_buffer_object_unique_ptr<T, U>(reinterpret_cast<T*>(glMapNamedBufferRange(get_resource_id(), offset * sizeof(T), len * sizeof(T), GL_MAP_WRITE_BIT | flags)),
													 get_resource_id(),
													 this,
													 { static_cast<std::size_t>(offset), len });
	}
	template <bool b = map_rw_allowed> 
	typename std::enable_if<b, mapped_buffer_object_unique_ptr<T, U>>::type
		map_rw(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return mapped_buffer_object_unique_ptr<T, U>(reinterpret_cast<T*>(glMapNamedBufferRange(get_resource_id(), offset * sizeof(T), len * sizeof(T), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | flags)),
													 get_resource_id(),
													 this,
													 { static_cast<std::size_t>(offset), len });
	}

	int size() const { return buffer_size; }

	void commit_range(int offset, std::size_t size) {
		int ps = page_size();
		int start_page = ps * (offset * sizeof(T) / ps);
		int pages = ps * (((offset + size) * sizeof(T) + ps - 1) / ps) - start_page;

		glNamedBufferPageCommitmentEXT(get_resource_id(), start_page, pages, true);
	}
	void uncommit_range(int offset, std::size_t size) {
		int ps = page_size();
		int start_page = ps * (offset * sizeof(T) / ps);
		int pages = ps * (((offset + size) * sizeof(T) + ps - 1) / ps) - start_page;

		glNamedBufferPageCommitmentEXT(get_resource_id(), start_page, pages, false);
	}

	static int page_size() {
		int n;
		glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &n);
		return n;
	}
};

template <typename S, BufferUsage::buffer_usage U1, typename T, BufferUsage::buffer_usage U2>
void operator>>(const buffer_object<T, U1> &lhs, buffer_object<S, U2> &rhs) { lhs.copy_to(rhs); }
template <typename S, BufferUsage::buffer_usage U1, typename T, BufferUsage::buffer_usage U2>
void operator<<(buffer_object<T, U1> &lhs, const buffer_object<S, U2> &rhs) { rhs.copy_to(lhs); }

template <typename BinderType>
class BufferObjectLayoutBinder {
private:
	using LayoutLocationType = layout_binding<BinderType>;
	using EmptyLayoutLocationType = layout_binding_none<BinderType>;

public:
	static void bind(unsigned int id, const LayoutLocationType &index, GLenum target) {
		if (index != EmptyLayoutLocationType()) glBindBufferBase(target, index, id);
	}
	static void unbind(const LayoutLocationType &index, GLenum target = 0) {
		if (index != EmptyLayoutLocationType()) glBindBufferBase(target, index, 0);
	}
};

template <typename Type, typename BinderType, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone, class LB = BufferObjectLayoutBinder<BinderType>>
class buffer_object_layout_bindable : public buffer_object<Type, U>, public shader_layout_bindable_resource<BinderType> {
protected:
	using Base = buffer_object<Type, U>;
	using LayoutBinder = LB;

protected:
	buffer_object_layout_bindable(buffer_object_layout_bindable &&m) = default;
	buffer_object_layout_bindable& operator=(buffer_object_layout_bindable &&m) = default;

	virtual ~buffer_object_layout_bindable() noexcept {}

	using Base::Base;

	using buffer_object<T, U>::bind;
	using buffer_object<T, U>::unbind;
};

}
}

#include "buffer_object_cast.h"
