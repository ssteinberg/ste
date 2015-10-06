// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "buffer_usage.h"

namespace StE {
namespace LLR {

struct IndirectDrawCommand {
	unsigned int count;
	unsigned int instance_count;
	unsigned int first_index;
	unsigned int base_vertex;
	unsigned int base_instance;
};

class IndirectDrawBufferGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class IndirectDrawBuffer : public buffer_object<IndirectDrawCommand, U>, public IndirectDrawBufferGeneric {
private:
	using Base = buffer_object<T, U>;

public:
	IndirectDrawBuffer(IndirectDrawBuffer &&m) = default;
	IndirectDrawBuffer& operator=(IndirectDrawBuffer &&m) = default;

	IndirectDrawBuffer(std::size_t size) : Base(size) {}
	IndirectDrawBuffer(std::size_t size, const T *data) : Base(size, data) {}
	IndirectDrawBuffer(const std::vector<T> &data) : Base(data.size(), &data[0]) {}

	void bind() const final override { Binder::bind(id, GL_DRAW_INDIRECT_BUFFER); };
	void unbind() const final override { Binder::unbind(GL_DRAW_INDIRECT_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRIndirectDrawBufferObject; }
};

}
}
