//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <command_recorder.hpp>

#include <camera.hpp>
#include <array.hpp>

#include <glm/gtx/dual_quaternion.hpp>

namespace ste {
namespace graphics {

class transforms_ring_buffers {
private:
	struct view_data {
		glm::dualquat view_transform;
		glm::dualquat inverse_view_transform;
		glm::vec4 eye_position;
	};
	struct proj_data {
		glm::vec4 proj_xywz;
		glm::uvec2 backbuffer_size;
		float tan_half_fovy;
		float aspect;
	};

	using view_buffer_type = gl::array<view_data>;
	using proj_buffer_type = gl::array<proj_data>;

private:
	view_buffer_type view_buffer;
	proj_buffer_type proj_buffer;

public:
	transforms_ring_buffers(const ste_context &ctx)
		: view_buffer(ctx, 1, gl::buffer_usage::storage_buffer),
		proj_buffer(ctx, 1, gl::buffer_usage::storage_buffer)
	{}

	void update_view_data(gl::command_recorder &recorder,
						  const camera<float> &c) {
		auto p = c.get_position();

		view_data v;
		v.view_transform = c.view_transform_dquat();
		v.inverse_view_transform = glm::inverse(v.view_transform);
		v.eye_position = glm::vec4{ p.x, p.y, p.z, .0f };

		recorder << view_buffer.overwrite_cmd(0, v);
	}

	void update_proj_data(gl::command_recorder &recorder,
						  float fovy, float aspect, float fnear, const glm::uvec2 backbuffer_size) {
		proj_data p;

		float tanHalfFovy = glm::tan(fovy * .5f);
		float one_over_tan_half_fovy = 1.f / tanHalfFovy;
		float one_over_aspect_tan_half_fovy = 1.f / (aspect * tanHalfFovy);

		p.proj_xywz = { one_over_aspect_tan_half_fovy, one_over_tan_half_fovy, fnear, -1.f };
		p.backbuffer_size = backbuffer_size;
		p.tan_half_fovy = tanHalfFovy;
		p.aspect = aspect;

		recorder << proj_buffer.overwrite_cmd(0, p);
	}

	auto& get_view_buffer() const { return view_buffer; }
	auto& get_proj_buffer() const { return proj_buffer; }
};

}
}
