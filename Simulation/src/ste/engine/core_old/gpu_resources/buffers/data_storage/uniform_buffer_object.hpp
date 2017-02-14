// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <buffer_object.hpp>

namespace StE {
namespace Core {

class uniform_buffer_object_layout_binding_type {};
using uniform_layout_binding = layout_binding<uniform_buffer_object_layout_binding_type>;
uniform_layout_binding inline operator "" _uniform_idx(unsigned long long int i) { return uniform_layout_binding(i); }

class uniform_buffer_generic {
public:
	virtual void bind(const layout_binding<uniform_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<uniform_buffer_object_layout_binding_type> &) const = 0;

	virtual ~uniform_buffer_generic() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class uniform_buffer_object : public buffer_object_layout_bindable<Type, uniform_buffer_object_layout_binding_type, U>, public uniform_buffer_generic {
private:
	using Base = buffer_object_layout_bindable<Type, uniform_buffer_object_layout_binding_type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	uniform_buffer_object(uniform_buffer_object &&m) = default;
	uniform_buffer_object& operator=(uniform_buffer_object &&m) = default;

	using Base::Base;

	void bind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::bind(Base::get_resource_id(), sampler, GL_UNIFORM_BUFFER); };
	void unbind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::unbind(sampler, GL_UNIFORM_BUFFER); };
	void bind_range(const typename Base::LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_UNIFORM_BUFFER, offset, size); }

	core_resource_type resource_type() const override { return core_resource_type::UniformBufferObject; }
};

}
}
