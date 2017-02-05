// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "signal.hpp"

#include "texture_2d.hpp"
#include "texture_2d_array.hpp"
#include "framebuffer_object.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class deferred_gbuffer {
private:
	std::unique_ptr<Core::texture_2d_array> gbuffer{ nullptr };
	std::unique_ptr<Core::texture_2d> depth_target{ nullptr };
	std::unique_ptr<Core::texture_2d> backface_depth_target{ nullptr };
	std::unique_ptr<Core::texture_2d> downsampled_depth_target{ nullptr };
	Core::framebuffer_object fbo;
	Core::framebuffer_object backface_fbo;

	glm::ivec2 size;
	int depth_buffer_levels;

	signal<> depth_target_modified_signal;

public:
	deferred_gbuffer(glm::ivec2 size, int depth_buffer_levels) : depth_buffer_levels(depth_buffer_levels) {
		resize(size);
	}

	void resize(glm::ivec2 size);
	void clear();

	auto* get_gbuffer() const { return gbuffer.get(); }

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
