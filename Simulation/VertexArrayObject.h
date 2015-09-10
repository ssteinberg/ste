// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class VertexArrayObject {
private:
	GLuint buffer_id;

public:
	VertexArrayObject(VertexArrayObject &&m) = default;
	VertexArrayObject(const VertexArrayObject &c) = delete;
	VertexArrayObject& operator=(VertexArrayObject &&m) = default;
	VertexArrayObject& operator=(const VertexArrayObject &c) = delete;

	VertexArrayObject() { glGenVertexArrays(1, &buffer_id); }
	virtual ~VertexArrayObject() { glDeleteVertexArrays(1, &buffer_id); }

	void bind() const {
		glBindVertexArray(buffer_id);
	}

	//void bind_vertex_attrib_to_(int index, std::size_t size, std::size_t stride, GLenum data_type, bool normalized, void *p) { glVertexAttribPointer(index, size, data_type, normalized, stride, p); }

	static void enable_vertex_attrib_array(int index) { glEnableVertexAttribArray(index); }
	static void disable_vertex_attrib_array(int index) { glDisableVertexAttribArray(index); }

protected:
	GLuint get_buffer_id() const { return buffer_id; }
};

}
}
