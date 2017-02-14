// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <buffer_object.hpp>
#include <buffer_usage.hpp>

namespace StE {
namespace Core {

struct indirect_dispatch_command {
	std::uint32_t num_groups_x;
	std::uint32_t num_groups_y;
	std::uint32_t num_groups_z;
};

class indirect_dispatch_buffer_object_generic {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~indirect_dispatch_buffer_object_generic() noexcept {}
};

template <BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class indirect_dispatch_buffer_object : public buffer_object<indirect_dispatch_command, U>, public indirect_dispatch_buffer_object_generic {
private:
	using Base = buffer_object<indirect_dispatch_command, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	indirect_dispatch_buffer_object(indirect_dispatch_buffer_object &&m) = default;
	indirect_dispatch_buffer_object& operator=(indirect_dispatch_buffer_object &&m) = default;

	using Base::Base;

	void bind() const final override { Base::Binder::bind(get_resource_id(), GL_DISPATCH_INDIRECT_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_DISPATCH_INDIRECT_BUFFER); };
	void bind_range(const LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_DISPATCH_INDIRECT_BUFFER, offset, size); }

	core_resource_type resource_type() const override { return core_resource_type::IndirectDispatchBufferObject; }
};

}
}
