// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <vector>

#include "VertexArrayObject.h"

namespace StE {
namespace LLR {

class VertexBufferObject {
private:
	GLuint buffer_id;
	mutable int buffer_type;
	std::vector<unsigned char> data;

	bool data_uploaded;

	void delete_vbo() {
		glDeleteBuffers(1, &buffer_id);
	}

public:
	VertexBufferObject(VertexBufferObject &&m) = default;
	VertexBufferObject(const VertexBufferObject &c) = delete;
	VertexBufferObject& operator=(VertexBufferObject &&m) = default;
	VertexBufferObject& operator=(const VertexBufferObject &c) = delete;

	VertexBufferObject(int size = 0) {
		data_uploaded = false;
		glGenBuffers(1, &buffer_id);
		data.reserve(size);
	}
	virtual ~VertexBufferObject() { delete_vbo(); }

	void* map(int usage_hint) {
		if (!data_uploaded) return nullptr;
		void* p = glMapBuffer(buffer_type, usage_hint);
		return p;
	}

	void* map(int usage_hint, unsigned int offset, unsigned int len) {
		if (!data_uploaded) return nullptr;
		void* p = glMapBufferRange(buffer_type, offset, len, usage_hint);
		return p;
	}

	void unmap() { glUnmapBuffer(buffer_type); }

	void bind(int type) const {
		buffer_type = type;
		glBindBuffer(buffer_type, buffer_id);
	}

	void bind_vertex_buffer_to_array(int index, unsigned int offset, unsigned int stride, int format_elements_count, GLenum format_type, bool normalized) {
		VertexArrayObject::enable_vertex_attrib_array(index);
		glBindVertexBuffer(index, buffer_id, offset, stride);
		glVertexAttribFormat(index, format_elements_count, format_type, normalized, 0);
		glVertexAttribBinding(index, index);
	}

	void upload(int type, int usage_hint) {
		bind(type);
		glBufferData(buffer_type, data.size(), &data[0], usage_hint);
		data_uploaded = true;
		data.clear();
	}

	void append(void* p, unsigned int size) {
		data.insert(data.end(), (unsigned char*)p, (unsigned char*)p + size);
	}

	std::size_t size() { return data.size(); }

	operator void*() {
		if (data_uploaded) return nullptr;
		return (void*)data[0];
	}

	operator const void*() const {
		if (data_uploaded) return nullptr;
		return (void*)data[0];
	}

protected:
	GLuint get_buffer_id() const { return buffer_id; }
};

}
}
