// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <bindable_resource.hpp>
#include <vertex_buffer_object.hpp>
#include <vertex_buffer_descriptor.hpp>

#include <gl_current_context.hpp>

#include <map>
#include <memory>

namespace StE {
namespace Core {

class vertex_array_object;

class vertex_array_object_allocator : public generic_resource_allocator {
public:
	generic_resource::type allocate() override final {
		GLuint id;
		glCreateVertexArrays(1, &id);
		return id;
	}
	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteVertexArrays(1, &id);
			id = 0;
		}
	}
};

class vertex_array_object_binder {
public:
	static void bind(generic_resource::type id) {
		GL::gl_current_context::get()->bind_vertex_array(id);
	}
	static void unbind() {
		GL::gl_current_context::get()->bind_vertex_array(0);
	}
};

class vertex_array_attrib_binder {
private:
	friend class vertex_array_object;

	int attrib_index;
	vertex_array_object *vao;

	vertex_array_attrib_binder(int index, vertex_array_object *vao) : attrib_index(index), vao(vao) {}
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

class vertex_array_object : public bindable_resource<vertex_array_object_allocator, vertex_array_object_binder> {
	using Base = bindable_resource<vertex_array_object_allocator, vertex_array_object_binder>;

private:
	friend class vertex_array_attrib_binder;

	using attrib_binding_map_type = std::unordered_map<int, const vertex_buffer_object_generic*>;

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
	vertex_array_object() {}
	vertex_array_object(vertex_array_object &&m) = default;
	vertex_array_object& operator=(vertex_array_object &&m) = default;

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
