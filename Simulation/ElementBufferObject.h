// StE
// � Shlomi Steinberg, 2015

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

	virtual ~ElementBufferObjectGeneric() noexcept {}
};

template <typename T = unsigned, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class ElementBufferObject : public buffer_object<T, U>, public ElementBufferObjectGeneric {
private:
	using Base = buffer_object<T, U>;

	static_assert(std::is_arithmetic<T>::value, "T must be of scalar type.");

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	ElementBufferObject(ElementBufferObject &&m) = default;
	ElementBufferObject& operator=(ElementBufferObject &&m) = default;

	using Base::Base;

	void bind() const final override { Base::Binder::bind(Base::get_resource_id(), GL_ELEMENT_ARRAY_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_ELEMENT_ARRAY_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRElementBufferObject; }
};

}
}
