// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "VertexBufferDescriptor.h"

#include <type_traits>
#include <vector>

namespace StE {
namespace LLR {

class VertexBufferObjectGeneric : public bindable_generic_resource {
public:
	virtual VertexBufferDescriptor *data_descriptor() = 0;
};

class vertex_buffer_attrib_binder {
private:
	template <typename, class, BufferUsage::buffer_usage>
	friend class VertexBufferObject;
	friend class VertexArrayObject;

	int binding_index;
	std::size_t offset, size;
	std::shared_ptr<VertexBufferObjectGeneric> vbo;

	vertex_buffer_attrib_binder(int index, const std::shared_ptr<VertexBufferObjectGeneric> &vbo, std::size_t offset, std::size_t size) : 
		binding_index(index), vbo(vbo), offset(offset), size(size) {}
	vertex_buffer_attrib_binder(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder& operator=(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder(const vertex_buffer_attrib_binder &m) = delete;
	vertex_buffer_attrib_binder& operator=(const vertex_buffer_attrib_binder &m) = delete;

public:
	~vertex_buffer_attrib_binder() {}
};

template <typename Type, class Descriptor, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class VertexBufferObject : public buffer_object<Type, U>, public VertexBufferObjectGeneric, protected std::enable_shared_from_this<VertexBufferObject<Type, Descriptor, U>> {
private:
	using buffer_object<Type, U>::bind;
	using buffer_object<Type, U>::unbind;

	static_assert(std::is_base_of<VertexBufferDescriptor, Descriptor>::value, "Descriptor must inherit from VertexBufferDescriptor");

private:
	Descriptor descriptor;

public:
	VertexBufferObject(VertexBufferObject &&m) = default;
	VertexBufferObject& operator=(VertexBufferObject &&m) = default;

	VertexBufferObject(std::size_t size) : buffer_object(size) {}
	VertexBufferObject(std::size_t size, const T *data) : buffer_object(size, data) {}
	VertexBufferObject(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {}

	void bind() const override { Binder::bind(id, GL_ARRAY_BUFFER); }
	void unbind() const override { Binder::unbind(GL_ARRAY_BUFFER); }

	virtual VertexBufferDescriptor *data_descriptor() { return &descriptor; }

	vertex_buffer_attrib_binder operator[](int index) { return vertex_buffer_attrib_binder(index, shared_from_this(), 0, sizeof(T)); }
	vertex_buffer_attrib_binder binder(int index, std::size_t offset) { return vertex_buffer_attrib_binder(index, shared_from_this(), offset, sizeof(T)); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRVertexBufferObject; }
};

}
}
