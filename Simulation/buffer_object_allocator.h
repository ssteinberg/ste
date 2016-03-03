#pragma once

#include "stdafx.h"
#include "resource_allocator.h"

namespace StE {
namespace LLR {

template <typename T, BufferUsage::buffer_usage U>
class buffer_object_immutable_storage_allocator : public generic_resource_immutable_storage_allocator<std::size_t, const T*> {
	using Base = generic_resource_immutable_storage_allocator<std::size_t, const T*>;
	
protected:
	unsigned allocate() override final {
		GLuint id;
		glCreateBuffers(1, &id);
		return id;
	}

	static void deallocate(unsigned &id) {
		if (id)
			glDeleteBuffers(1, &id);
		id = 0;
	}

public:
	void allocate_storage(unsigned id, std::size_t buffer_size, const T *data) override final {
		GLenum flags = static_cast<GLenum>(U);
		glNamedBufferStorage(id, sizeof(T)*buffer_size, data, flags);
	}
};

}
}
