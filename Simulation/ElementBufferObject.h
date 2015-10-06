// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"

#include <type_traits>
#include <vector>

namespace StE {
namespace LLR {

class ElementBufferObjectGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

template <typename T = unsigned int, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class ElementBufferObject : public buffer_object<T, U>, public ElementBufferObjectGeneric {
private:
	using Base = buffer_object<T, U>;

	static_assert(std::is_arithmetic<T>::value, "T must be of scalar type.");

public:
	ElementBufferObject(ElementBufferObject &&m) = default;
	ElementBufferObject& operator=(ElementBufferObject &&m) = default;

	ElementBufferObject(std::size_t size) : buffer_object(size) {}
	ElementBufferObject(std::size_t size, const T *data) : buffer_object(size, data) {}
	ElementBufferObject(const std::vector<T> &data) : buffer_object(data.size(), &data[0]) {}

	void bind() const final override { Binder::bind(id, GL_ELEMENT_ARRAY_BUFFER); };
	void unbind() const final override { Binder::unbind(GL_ELEMENT_ARRAY_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRElementBufferObject; }
};

}
}
