// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "VertexBufferDescriptor.h"

#include <type_traits>
#include <vector>

namespace StE {
namespace LLR {
	
template <typename, class, BufferUsage::buffer_usage>
class VertexBufferObject;

class VertexBufferObjectGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
	virtual const VertexBufferDescriptor *data_descriptor() const = 0;

	virtual ~VertexBufferObjectGeneric() noexcept {}
};

template <typename T, class D, BufferUsage::buffer_usage U>
class vertex_buffer_attrib_binder {
private:
	template <typename, class, BufferUsage::buffer_usage>
	friend class VertexBufferObject;
	friend class VertexArrayObject;

	int binding_index;
	std::size_t offset, size;
	const VertexBufferObject<T, D, U> *vbo;

	vertex_buffer_attrib_binder(int index, const VertexBufferObject<T, D, U> *vbo, std::size_t offset, std::size_t size) :
		binding_index(index), offset(offset), size(size), vbo(vbo) {}
	vertex_buffer_attrib_binder(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder& operator=(vertex_buffer_attrib_binder &&m) = default;
	vertex_buffer_attrib_binder(const vertex_buffer_attrib_binder &m) = delete;
	vertex_buffer_attrib_binder& operator=(const vertex_buffer_attrib_binder &m) = delete;

public:
	~vertex_buffer_attrib_binder() {}
};

template <typename Type, class Descriptor, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class VertexBufferObject : public buffer_object<Type, U>, public VertexBufferObjectGeneric {
private:
	using Base = buffer_object<Type, U>;
	using buffer_object<Type, U>::bind;
	using buffer_object<Type, U>::unbind;

	static_assert(std::is_base_of<VertexBufferDescriptor, Descriptor>::value, "Descriptor must inherit from VertexBufferDescriptor");

	ALLOW_BUFFER_OBJECT_CASTS;

private:
	Descriptor desc;

public:
	VertexBufferObject(VertexBufferObject &&m) = default;
	VertexBufferObject& operator=(VertexBufferObject &&m) = default;

	using Base::Base;

	~VertexBufferObject() noexcept {}

	virtual const VertexBufferDescriptor *data_descriptor() const override { return &desc; }

	vertex_buffer_attrib_binder<Type, Descriptor, U> operator[](int index) { return vertex_buffer_attrib_binder<Type, Descriptor, U>(index, this, 0, sizeof(Type)); }
	vertex_buffer_attrib_binder<Type, Descriptor, U> binder(int index, std::size_t offset) { return vertex_buffer_attrib_binder<Type, Descriptor, U>(index, this, offset, sizeof(Type)); }

	void bind() const final override { Base::Binder::bind(Base::get_resource_id(), GL_ARRAY_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_ARRAY_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRVertexBufferObject; }
};

}
}
