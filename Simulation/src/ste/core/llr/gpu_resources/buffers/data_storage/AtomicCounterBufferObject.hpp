// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.hpp"

namespace StE {
namespace LLR {

class atomic_buffer_object_layout_binding_type {};
using atomic_layout_binding = layout_binding<atomic_buffer_object_layout_binding_type>;
atomic_layout_binding inline operator "" _atomic_idx(unsigned long long int i) { return atomic_layout_binding(i); }

class AtomicCounterBufferObjectGeneric {
public:
	virtual void bind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;

	virtual ~AtomicCounterBufferObjectGeneric() noexcept {}
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class AtomicCounterBufferObject : public buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>, public AtomicCounterBufferObjectGeneric {
private:
	using Base = buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	AtomicCounterBufferObject(AtomicCounterBufferObject &&m) = default;
	AtomicCounterBufferObject& operator=(AtomicCounterBufferObject &&m) = default;

	using Base::Base;

	void bind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::bind(Base::get_resource_id(), sampler, GL_ATOMIC_COUNTER_BUFFER); };
	void unbind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::unbind(sampler, GL_ATOMIC_COUNTER_BUFFER); };
	void bind_range(const typename Base::LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_ATOMIC_COUNTER_BUFFER, offset, size); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRAtomicCounterBufferObject; }
};

}
}
