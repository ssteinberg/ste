// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "bindable_resource.h"

#include "buffer_usage.h"
#include "BufferMappedStorage.h"

#include "shader_layout_bindable_resource.h"
#include "layout_binding.h"

#include <memory>
#include <list>

#include <type_traits>

namespace StE {
namespace LLR {

class BufferObjectAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateBuffers(1, &id); return id; }
	static void deallocate(unsigned int &id) { if (id) glDeleteBuffers(1, reinterpret_cast<GLuint*>(&id)); id = 0; }
};

class BufferObjectBinder {
public:
	static void bind(unsigned int id, GLenum target) {
		glBindBuffer(target, id);
	}
	static void unbind(GLenum target = 0) {
		glBindBuffer(target, 0);
	}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class buffer_object : public bindable_resource<BufferObjectAllocator, BufferObjectBinder, GLenum> {
protected:
	static constexpr BufferUsage::buffer_usage access_usage = U;
	using T = Type;

public:
	static constexpr bool map_read_allowed = !!(access_usage & BufferUsage::BufferUsageMapRead);
	static constexpr bool map_write_allowed = !!(access_usage & BufferUsage::BufferUsageMapWrite);
	static constexpr bool map_rw_allowed = map_read_allowed && map_write_allowed;

protected:
	std::size_t buffer_size;

	buffer_object(std::size_t size) : buffer_object(size, nullptr) {};
	buffer_object(std::size_t size, const T *data) : buffer_size(size) { glNamedBufferStorage(id, sizeof(T)*size, data, static_cast<GLenum>(access_usage)); }
	buffer_object(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {};

	using bindable_resource<BufferObjectAllocator, BufferObjectBinder, GLenum>::bind;
	using bindable_resource<BufferObjectAllocator, BufferObjectBinder, GLenum>::unbind;

public:
	buffer_object(buffer_object &&t) = default;
	buffer_object &operator=(buffer_object &&t) = default;

	void clear(const gli::format format, const void *data) {
		auto glf = opengl::gl_translate_format(format);
		glClearNamedBufferData(id, glf.Internal, glf.External, glf.Type, data);
	}
	void clear(const gli::format format, const void *data, int offset, int size) {
		auto glf = opengl::gl_translate_format(format);
		glClearNamedBufferSubData(id, glf.Internal, offset, size, glf.External, glf.Type, data);
	}
	void invalidate_data() { glInvalidateBufferData(id); }
	void invalidate_data(int offset, int length) { glInvalidateBufferSubData(id, offset, length); }

	template <typename S>
	void copy_to(buffer_object<S> &bo) const { glCopyNamedBufferSubData(id, bo.id, 0, 0, buffer_size * sizeof(T)); }
	template <typename S>
	void copy_to(buffer_object<S> &bo, int read_offset, int write_offset, std::size_t size) const {
		glCopyNamedBufferSubData(id, bo.id, read_offset, write_offset, size * sizeof(T)); 
	}

	template <typename S>
	void operator<<(const buffer_object<S> &bo) { bo.copy_to(*this); }
	template <typename S>
	void operator>>(buffer_object<S> &bo) const { copy_to(bo); }

	template <bool b = map_read_allowed> 
	typename std::enable_if<b, BufferMappedStorage<const T>>::type 
		map_read(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return BufferMappedStorage<const T>(reinterpret_cast<const T*>(glMapNamedBufferRange(id, offset, len * sizeof(T), GL_MAP_READ_BIT | flags)), id);
	}
	template <bool b = map_write_allowed> 
	typename std::enable_if<b, BufferMappedStorage<T>>::type 
		map_write(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return BufferMappedStorage<T>(reinterpret_cast<T*>(glMapNamedBufferRange(id, offset, len * sizeof(T), GL_MAP_WRITE_BIT | flags)), id);
	}
	template <bool b = map_rw_allowed> 
	typename std::enable_if<b, BufferMappedStorage<T>>::type 
		map_rw(std::size_t len, int offset = 0, BufferUsage::buffer_mapping flags = BufferUsage::BufferMapNone) {
		return BufferMappedStorage<T>(reinterpret_cast<T*>(glMapNamedBufferRange(id, offset, len * sizeof(T), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | flags)), id);
	}

	int size() const { return buffer_size; }
};

template <typename BinderType>
class BufferObjectLayoutBinder {
private:
	using LayoutLocationType = layout_binding<BinderType>;

public:
	static void bind(unsigned int id, const LayoutLocationType &index, GLenum target) {
		if (index != layout_location_empty) glBindBufferBase(target, index, id);
	}
	static void unbind(const LayoutLocationType &index, GLenum target = 0) {
		if (index != layout_location_empty) glBindBufferBase(target, index, 0);
	}
};

template <typename Type, typename BinderType, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone, class LB = BufferObjectLayoutBinder<BinderType>>
class buffer_object_layout_bindable : public buffer_object<Type, U>, virtual public shader_layout_bindable_resource<BinderType> {
protected:
	using LayoutBinder = LB;

protected:
	buffer_object_layout_bindable(buffer_object_layout_bindable &&m) = default;
	buffer_object_layout_bindable& operator=(buffer_object_layout_bindable &&m) = default;

	buffer_object_layout_bindable(std::size_t size) : buffer_object(size) {}
	buffer_object_layout_bindable(std::size_t size, const T *data) : buffer_object(size, data) {}
	buffer_object_layout_bindable(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {}

	using buffer_object<T, U>::bind;
	using buffer_object<T, U>::unbind;
};

}
}
