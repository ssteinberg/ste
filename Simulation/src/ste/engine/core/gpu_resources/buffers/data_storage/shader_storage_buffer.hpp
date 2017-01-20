// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "buffer_object.hpp"

namespace StE {
namespace Core {

class shader_storage_buffer_object_layout_binding_type {};
using shader_storage_layout_binding = layout_binding<shader_storage_buffer_object_layout_binding_type>;
shader_storage_layout_binding inline operator "" _storage_idx(unsigned long long int i) { return shader_storage_layout_binding(i); }

class shader_storage_buffer_generic {
public:
	virtual void bind(const layout_binding<shader_storage_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<shader_storage_buffer_object_layout_binding_type> &) const = 0;

	virtual ~shader_storage_buffer_generic() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class shader_storage_buffer : public buffer_object_layout_bindable<Type, shader_storage_buffer_object_layout_binding_type, U>, public shader_storage_buffer_generic {
private:
	using Base = buffer_object_layout_bindable<Type, shader_storage_buffer_object_layout_binding_type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	shader_storage_buffer(shader_storage_buffer &&m) = default;
	shader_storage_buffer& operator=(shader_storage_buffer &&m) = default;

	using Base::Base;

	void bind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::bind(Base::get_resource_id(), sampler, GL_SHADER_STORAGE_BUFFER); };
	void unbind(const typename Base::LayoutLocationType &sampler) const final override { Base::LayoutBinder::unbind(sampler, GL_SHADER_STORAGE_BUFFER); };
	void bind_range(const typename Base::LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_SHADER_STORAGE_BUFFER, offset, size); }

	core_resource_type resource_type() const override { return core_resource_type::ShaderStorageBufferObject; }
};

}
}
