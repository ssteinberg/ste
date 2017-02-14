// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <buffer_object.hpp>
#include <buffer_usage.hpp>

namespace StE {
namespace Core {

struct indirect_multi_draw_elements_command {
	std::uint32_t count;
	std::uint32_t instance_count;
	std::uint32_t first_index;
	std::uint32_t base_vertex;
	std::uint32_t base_instance;
};

struct indirect_draw_arrays_command {
	std::uint32_t count;
	std::uint32_t prim_count;
	std::uint32_t first_index;
	std::uint32_t base_instance;
};

class indirect_draw_buffer_generic {
public:
	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual ~indirect_draw_buffer_generic() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class indirect_draw_buffer_object : public buffer_object<Type, U>, public indirect_draw_buffer_generic {
private:
	using Base = buffer_object<Type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	indirect_draw_buffer_object(indirect_draw_buffer_object &&m) = default;
	indirect_draw_buffer_object& operator=(indirect_draw_buffer_object &&m) = default;

	using Base::Base;

	void bind() const final override { Base::Binder::bind(Base::get_resource_id(), GL_DRAW_INDIRECT_BUFFER); };
	void unbind() const final override { Base::Binder::unbind(GL_DRAW_INDIRECT_BUFFER); };

	core_resource_type resource_type() const override { return core_resource_type::IndirectDrawBufferObject; }
};

}
}
