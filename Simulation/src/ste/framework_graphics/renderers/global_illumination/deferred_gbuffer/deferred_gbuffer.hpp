// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "signal.hpp"

#include "texture_2d.hpp"
#include "framebuffer_object.hpp"

#include "buffer_usage.hpp"
#include "shader_storage_buffer.hpp"

#include "gl_current_context.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class deferred_gbuffer {
private:
	struct g_buffer_element {
		glm::mat2x4 data;
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t virt_size = 2147483648 / 2;

	using gbuffer_type = Core::shader_storage_buffer<g_buffer_element, usage>;

private:
	std::unique_ptr<Core::texture_2d> depth_target{ nullptr };
	std::unique_ptr<Core::texture_2d> backface_depth_target{ nullptr };
	std::unique_ptr<Core::texture_2d> downsampled_depth_target{ nullptr };
	Core::framebuffer_object fbo;
	Core::framebuffer_object backface_fbo;

	gbuffer_type gbuffer;

	glm::ivec2 size;
	int depth_buffer_levels;

	static std::size_t virtual_gbuffer_size() {
		return virt_size / gbuffer_type::page_size() / sizeof(g_buffer_element) * gbuffer_type::page_size();
	}

	signal<> depth_target_modified_signal;

public:
	deferred_gbuffer(glm::ivec2 size, int depth_buffer_levels) : gbuffer(virtual_gbuffer_size()),
																 depth_buffer_levels(depth_buffer_levels) {
		resize(size);
	}

	void resize(glm::ivec2 size);
	void clear();

	void bind_gbuffer() const;

	auto& get_size() const { return size; }
	
	auto* get_depth_target() const { return depth_target.get(); }
	auto* get_backface_depth_target() const { return backface_depth_target.get(); }
	auto* get_downsampled_depth_target() const { return downsampled_depth_target.get(); }
	auto* get_fbo() const { return &fbo; }
	auto* get_backface_fbo() const { return &backface_fbo; }

	auto& get_depth_target_modified_signal() const { return depth_target_modified_signal; }
};

}
}
