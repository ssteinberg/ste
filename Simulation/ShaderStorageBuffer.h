// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "buffer_object.h"

namespace StE {
namespace LLR {

class shader_storage_buffer_object_layout_binding_type {};
using shader_storage_layout_binding = layout_binding<shader_storage_buffer_object_layout_binding_type>;
shader_storage_layout_binding inline operator "" _storage_idx(unsigned long long int i) { return shader_storage_layout_binding(i); }

class ShaderStorageBufferGeneric {
public:
	virtual void bind(const layout_binding<shader_storage_buffer_object_layout_binding_type> &) const = 0;
	virtual void unbind(const layout_binding<shader_storage_buffer_object_layout_binding_type> &) const = 0;

	virtual ~ShaderStorageBufferGeneric() noexcept {}
};

template <typename Type, BufferUsage::buffer_usage U = BufferUsage::BufferUsageNone>
class ShaderStorageBuffer : public buffer_object_layout_bindable<Type, shader_storage_buffer_object_layout_binding_type, U>, public ShaderStorageBufferGeneric {
private:
	using Base = buffer_object_layout_bindable<Type, shader_storage_buffer_object_layout_binding_type, U>;

	ALLOW_BUFFER_OBJECT_CASTS;

public:
	ShaderStorageBuffer(ShaderStorageBuffer &&m) = default;
	ShaderStorageBuffer& operator=(ShaderStorageBuffer &&m) = default;

	// This causes:
	//	Internal error loop: assertion failed: find_seq_in_lookup_table: seq_number not found (shared/cfe/edgcpfe/il.c, line 3997)
	//	With ICC 16.0
	//using Base::Base;
	template <typename T2>
	ShaderStorageBuffer(const buffer_object<T2, U> &t) : Base(t) {}
	ShaderStorageBuffer(const ShaderStorageBuffer &t) : Base(t) {}
	ShaderStorageBuffer(std::size_t size) : Base(size) {}
	ShaderStorageBuffer(std::size_t size, const T *data) : Base(size, data) {}
	ShaderStorageBuffer(const std::vector<T> &data) : Base(data.size(), &data[0]) {}

	void bind(const LayoutLocationType &sampler) const final override { LayoutBinder::bind(get_resource_id(), sampler, GL_SHADER_STORAGE_BUFFER); };
	void unbind(const LayoutLocationType &sampler) const final override { LayoutBinder::unbind(sampler, GL_SHADER_STORAGE_BUFFER); };
	void bind_range(const LayoutLocationType &sampler, int offset, std::size_t size) const { Base::bind_range(sampler, GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), size * sizeof(T)); }

	llr_resource_type resource_type() const override { return llr_resource_type::LLRShaderStorageBufferObject; }
};

}
}
