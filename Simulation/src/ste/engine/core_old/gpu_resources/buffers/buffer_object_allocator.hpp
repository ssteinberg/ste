#pragma once

#include <stdafx.hpp>
#include <resource_allocator.hpp>

namespace StE {
namespace Core {

template <typename T, BufferUsage::buffer_usage U>
class buffer_object_immutable_storage_allocator : public generic_resource_immutable_storage_allocator<std::size_t, const T*> {
	using Base = generic_resource_immutable_storage_allocator<std::size_t, const T*>;
	
public:
	generic_resource::type allocate() override final {
		GLuint id;
		glCreateBuffers(1, &id);
		return id;
	}

	static void deallocate(generic_resource::type &id) {
		if (id) {
			glDeleteBuffers(1, &id);
			id = 0;
		}
	}

	void allocate_storage(generic_resource::type id, std::size_t buffer_size, const T *data) override final {
		GLenum flags = static_cast<GLenum>(U);
		glNamedBufferStorage(id, sizeof(T)*buffer_size, data, flags);
	}
};

}
}
