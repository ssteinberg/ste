// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"
#include "buffer_usage.h"

namespace StE {
namespace LLR {

struct IndirectDispatchCommand {
	unsigned int count;
	unsigned int instance_count;
	unsigned int first_index;
	unsigned int base_vertex;
	unsigned int base_instance;
};

class IndirectDispatchBufferObjectGeneric {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~IndirectDispatchBufferObjectGeneric() noexcept {}
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class IndirectDispatchBufferObject : public buffer_object<IndirectDispatchCommand, U>, public IndirectDispatchBufferObjectGeneric {
private:
	using Base = buffer_object<IndirectDispatchCommand, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	IndirectDispatchBufferObject(IndirectDispatchBufferObject &&m) = default;
	IndirectDispatchBufferObject& operator=(IndirectDispatchBufferObject &&m) = default;

	using Base::Base;

	void bind() const final override { Binder::bind(get_resource_id(), GL_DRAW_INDIRECT_BUFFER); };
	void unbind() const final override { Binder::unbind(GL_DRAW_INDIRECT_BUFFER); };

	llr_resource_type resource_type() const override { return llr_resource_type::LLRIndirectDispatchBufferObject; }
};

}
}
