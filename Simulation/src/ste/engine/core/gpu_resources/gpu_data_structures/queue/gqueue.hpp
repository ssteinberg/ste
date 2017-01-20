// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "shader_storage_buffer.hpp"
#include "atomic_counter_buffer_object.hpp"

#include <type_traits>
#include <vector>

namespace StE {
namespace Core {

template <typename T, int Size>
class gqueue {
public:
	using value_type = T;

protected:
	struct node {
		std::int32_t valid;
		value_type data;
	};

private:
	using buffer_type = shader_storage_buffer<node>;
	using tail_buffer_type = shader_storage_buffer<GLuint>;

	static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

private:
	buffer_type buffer;
	tail_buffer_type tail;
	atomic_counter_buffer_object<> head;

public:
	gqueue() : buffer(Size), tail(std::vector<GLuint>({ 0 })), head(std::vector<GLuint>({ 0 })) {
		int clear = 0;
		buffer.clear(gli::format::FORMAT_R8_SINT, &clear);
	}

	void bind_buffer(int location) const {
		buffer.bind(buffer_type::LayoutLocationType(location));
		tail.bind(buffer_type::LayoutLocationType(location + 1));
		head.bind(atomic_counter_buffer_object<>::LayoutLocationType(location + 2));
	}
};

}
}
