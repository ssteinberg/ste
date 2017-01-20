// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "buffer_object.hpp"

namespace StE {
namespace Core {

class atomic_buffer_object_layout_binding_type {};
using atomic_layout_binding = layout_binding<atomic_buffer_object_layout_binding_type>;
atomic_layout_binding inline operator "" _atomic_idx(unsigned long long int i) { return atomic_layout_binding(i); }

class atomic_counter_buffer_object_generic {
public:
	virtual void bind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;

	virtual ~atomic_counter_buffer_object_generic() noexcept {}
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class atomic_counter_buffer_object : public buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>, public atomic_counter_buffer_object_generic {
private:
	using Base = buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	atomic_counter_buffer_object(atomic_counter_buffer_object &&m) = default;
	atomic_counter_buffer_object& operator=(atomic_counter_buffer_object &&m) = default;

	using Base::Base;

	void bind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::bind(Base::get_resource_id(), sampler, GL_ATOMIC_COUNTER_BUFFER); };
	void unbind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::unbind(sampler, GL_ATOMIC_COUNTER_BUFFER); };
	void bind_range(const typename Base::LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_ATOMIC_COUNTER_BUFFER, offset, size); }

	core_resource_type resource_type() const override { return core_resource_type::AtomicCounterBufferObject; }
};

}
}
