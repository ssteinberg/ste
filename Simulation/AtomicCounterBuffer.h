// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"

namespace StE {
namespace LLR {

class atomic_buffer_object_layout_binding_type {};
using atomic_layout_binding = layout_binding<atomic_buffer_object_layout_binding_type>;
atomic_layout_binding inline operator "" _atomic_idx(unsigned long long int i) { return atomic_layout_binding(i); }

class AtomicCounterBufferGeneric {
public:
	virtual void bind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<atomic_buffer_object_layout_binding_type> &) const = 0;
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class AtomicCounterBuffer : public buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>, public AtomicCounterBufferGeneric {
private:
	using Base = buffer_object_layout_bindable<GLuint, atomic_buffer_object_layout_binding_type, U>;

public:
	AtomicCounterBuffer(AtomicCounterBuffer &&m) = default;
	AtomicCounterBuffer& operator=(AtomicCounterBuffer &&m) = default;

	AtomicCounterBuffer(std::size_t size) : Base(size) {}
	AtomicCounterBuffer(std::size_t size, const T *data) : Base(size, data) {}
	AtomicCounterBuffer(const std::vector<T> &data) : Base(data.size(), &data[0]) {}

	void bind(const LayoutLocationType &sampler) const final override { LayoutBinder::bind(id, sampler, GL_ATOMIC_COUNTER_BUFFER); };
	void unbind(const LayoutLocationType &sampler) const final override { LayoutBinder::unbind(sampler, GL_ATOMIC_COUNTER_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRAtomicCounterBufferObject; }
};

}
}
