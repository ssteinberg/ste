// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "buffer_object.hpp"

namespace StE {
namespace Core {

class element_buffer_object_generic {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~element_buffer_object_generic() noexcept {}
};

template <typename T = std::uint32_t, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class element_buffer_object : public buffer_object<T, U>, public element_buffer_object_generic {
private:
	using Base = buffer_object<T, U>;

	static_assert(std::is_arithmetic<T>::value, "T must be of scalar type.");

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	element_buffer_object(element_buffer_object &&m) = default;
	element_buffer_object& operator=(element_buffer_object &&m) = default;

	using Base::Base;

	void bind() const final override { Base::Binder::bind(Base::get_resource_id(), GL_ELEMENT_ARRAY_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_ELEMENT_ARRAY_BUFFER); };

	core_resource_type resource_type() const override { return core_resource_type::ElementBufferObject; }
};

}
}
