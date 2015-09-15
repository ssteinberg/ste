// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "VertexBufferObject.h"

#include <type_traits>
#include <vector>

namespace StE {
namespace LLR {

class ElementBufferObjectGeneric : public bindable_generic_resource {};

template <typename T = unsigned int>
class ElementBufferObject : public VertexBufferObject<T, VBODescriptorWithTypes<T>::descriptor>, public ElementBufferObjectGeneric {
private:
	using vbo_type = VertexBufferObject<T, VBODescriptorWithTypes<T>::descriptor>;

	static_assert(std::is_arithmetic<T>::value, "Indices must be of scalar type.");

public:
	ElementBufferObject(ElementBufferObject &&m) = default;
	ElementBufferObject& operator=(ElementBufferObject &&m) = default;

	using vbo_type::VertexBufferObject;

	void bind() const override { Binder::bind(id, GL_ELEMENT_ARRAY_BUFFER); }
	void unbind() const override { unbind(); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRElementBufferObject; }
};

}
}
