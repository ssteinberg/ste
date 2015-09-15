// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "bindable_resource.h"
#include "VertexBufferObject.h"
#include "VertexBufferDescriptor.h"

#include <map>
#include <memory>

namespace StE {
namespace LLR {

class VertexArrayObjectAllocator : public llr_resource_stub_allocator {
public:
	static int allocate() { GLuint id;  glCreateVertexArrays(1, &id); return id; }
	static void deallocate(unsigned int &id) { glDeleteVertexArrays(1, reinterpret_cast<GLuint*>(&id)); id = 0; }
};

class VertexArrayObjectBinder {
public:
	static void bind(unsigned int id) {
		glBindVertexArray(id);
	}
	static void unbind() {
		glBindVertexArray(0);
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

	vertex_array_attrib_binder &operator=(vertex_buffer_attrib_binder &&binder) {
		attach(std::move(binder));
		return *this;
	}
	vertex_array_attrib_binder &operator = (void *p) {
		assert(!p && "p must be null to detach attrib binding");
		if (p == nullptr)
			detach();
		return *this;
	}
	void attach(vertex_buffer_attrib_binder &&binder);
	void detach();
};

class VertexArrayObject : public bindable_resource<VertexArrayObjectAllocator, VertexArrayObjectBinder> {
private:
	friend class vertex_array_attrib_binder;

	using attrib_binding_map_type = std::map<int, std::shared_ptr<VertexBufferObjectGeneric>>;

private:
	attrib_binding_map_type attrib_bindings;

	void detach(int attrib_index) {
		bind();
		glVertexAttribBinding(attrib_index, 0);

		attrib_bindings.erase(attrib_index);
	}

	void attach(int attrib_index, vertex_buffer_attrib_binder &&binder) {
		auto &ptr = binder.vbo;
		auto descriptor = ptr->data_descriptor();

		bind();
		enable_vertex_attrib_array(attrib_index);
 		glBindVertexBuffer(binder.binding_index, ptr->get_resource_id(), descriptor->offset_to_attrib(binder.binding_index), binder.size);
		glVertexAttribFormat(attrib_index, 
							 descriptor->attrib_element_count(binder.binding_index), 
							 descriptor->attrib_element_type(binder.binding_index), 
							 descriptor->attrib_element_normalized(binder.binding_index), 
							 binder.offset);
		glVertexAttribBinding(attrib_index, binder.binding_index);
		attrib_bindings[attrib_index] = ptr;
	}

public:
	VertexArrayObject() {}
	VertexArrayObject(VertexArrayObject &&m) = default;
	VertexArrayObject& operator=(VertexArrayObject &&m) = default;

	static void enable_vertex_attrib_array(int index) { glEnableVertexAttribArray(index); }
	static void disable_vertex_attrib_array(int index) { glDisableVertexAttribArray(index); }

	vertex_array_attrib_binder operator[](int index) { return vertex_array_attrib_binder(index, this); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRVertexArrayObject; }
};

void inline vertex_array_attrib_binder::attach(vertex_buffer_attrib_binder &&binder) { vao->attach(attrib_index, std::move(binder)); }
void inline vertex_array_attrib_binder::detach() { vao->detach(attrib_index); }

}
}
