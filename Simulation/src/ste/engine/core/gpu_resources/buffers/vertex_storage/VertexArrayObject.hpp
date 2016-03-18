// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "bindable_resource.hpp"
#include "VertexBufferObject.hpp"
#include "VertexBufferDescriptor.hpp"

#include "gl_current_context.hpp"

#include <map>
#include <memory>

namespace StE {
namespace Core {
	
class VertexArrayObject;

class VertexArrayObjectAllocator : public generic_resource_allocator {
public:
	GenericResource::type allocate() override final {
		GLuint id;
		glCreateVertexArrays(1, &id);
		return id;
	}
	static void deallocate(GenericResource::type &id) {
		if (id) {
			glDeleteVertexArrays(1, &id);
			id = 0;
		}
	}
};

class VertexArrayObjectBinder {
public:
	static void bind(GenericResource::type id) {
		gl_current_context::get()->bind_vertex_array(id);
	}
	static void unbind() {
		gl_current_context::get()->bind_vertex_array(0);
	}
};

class vertex_array_attrib_binder {
private:
	friend class VertexArrayObject;

	int attrib_index;
	VertexArrayObject *vao;

	vertex_array_attrib_binder(int index, VertexArrayObject *vao) : attrib_index(index), vao(vao) {}
	vertex_array_attrib_binder(vertex_array_attrib_binder &&m) = default;
	vertex_array_attrib_binder& operator=(vertex_array_attrib_binder &&m) = delete;
	vertex_array_attrib_binder(const vertex_array_attrib_binder &m) = delete;
	vertex_array_attrib_binder& operator=(const vertex_array_attrib_binder &m) = delete;

public:
	~vertex_array_attrib_binder() {}

	template <typename T, class D, BufferUsage::buffer_usage U>
	vertex_array_attrib_binder &operator=(vertex_buffer_attrib_binder<T, D, U> &&binder) {
		attach(std::move(binder));
		return *this;
	}
	vertex_array_attrib_binder &operator = (void *p) {
		assert(!p && "p must be null to detach attrib binding");
		if (p == nullptr)
			detach();
		return *this;
	}
	template <typename T, class D, BufferUsage::buffer_usage U>
	void attach(vertex_buffer_attrib_binder<T, D, U> &&binder);
	void detach();
};

class VertexArrayObject : public bindable_resource<VertexArrayObjectAllocator, VertexArrayObjectBinder> {
	using Base = bindable_resource<VertexArrayObjectAllocator, VertexArrayObjectBinder>;
	
private:
	friend class vertex_array_attrib_binder;

	using attrib_binding_map_type = std::map<int, const VertexBufferObjectGeneric*>;

private:
	attrib_binding_map_type attrib_bindings;

	void detach(int attrib_index) {
		bind();
		glVertexAttribBinding(attrib_index, 0);

		attrib_bindings.erase(attrib_index);
	}

	template <typename T, class D, BufferUsage::buffer_usage U>
	void attach(int attrib_index, vertex_buffer_attrib_binder<T, D, U> &&binder) {
		auto &ptr = binder.vbo;
		auto descriptor = ptr->data_descriptor();

		enable_vertex_attrib_array(attrib_index);
		glVertexArrayVertexBuffer(Base::get_resource_id(), 
								  binder.binding_index, 
								  ptr->get_resource_id(), 
								  descriptor->offset_to_attrib(binder.binding_index), 
								  binder.size);
		glVertexArrayAttribFormat(Base::get_resource_id(), 
								  attrib_index, 
								  descriptor->attrib_element_count(binder.binding_index), 
								  descriptor->attrib_element_type(binder.binding_index), 
								  descriptor->attrib_element_normalized(binder.binding_index), 
								  binder.offset);
		glVertexArrayAttribBinding(Base::get_resource_id(), 
								   attrib_index, 
								   binder.binding_index);
								   
		attrib_bindings[attrib_index] = ptr;
	}

public:
	VertexArrayObject() {}
	VertexArrayObject(VertexArrayObject &&m) = default;
	VertexArrayObject& operator=(VertexArrayObject &&m) = default;

	void enable_vertex_attrib_array(std::uint32_t index) { glEnableVertexArrayAttrib(Base::get_resource_id(), index); }
	void disable_vertex_attrib_array(std::uint32_t index) { glDisableVertexArrayAttrib(Base::get_resource_id(), index); }

	vertex_array_attrib_binder operator[](int index) { return vertex_array_attrib_binder(index, this); }

	core_resource_type resource_type() const override { return core_resource_type::VertexArrayObject; }
};

template <typename T, class D, BufferUsage::buffer_usage U>
void inline vertex_array_attrib_binder::attach(vertex_buffer_attrib_binder<T, D, U> &&binder) { vao->attach(attrib_index, std::move(binder)); }
void inline vertex_array_attrib_binder::detach() { vao->detach(attrib_index); }

}
}
