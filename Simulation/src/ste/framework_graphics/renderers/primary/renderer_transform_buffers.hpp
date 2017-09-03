//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <command_recorder.hpp>

#include <primary_renderer_camera.hpp>
#include <array.hpp>
#include <std430.hpp>

#include <glm/gtx/dual_quaternion.hpp>

namespace ste {
namespace graphics {

class renderer_transform_buffers {
private:
	struct view_data : gl::std430<gl::std430<glm::quat, glm::quat>, gl::std430<glm::quat, glm::quat>, glm::vec4> {
		void set_view_transform(const glm::dualquat &q) {
			get<0>().get<0>() = q.real;
			get<0>().get<1>() = q.dual;
		}
		auto view_transform() { return glm::dualquat{ get<0>().get<0>(), get<0>().get<0>() }; }

		void set_inverse_view_transform(const glm::dualquat &q) {
			get<1>().get<0>() = q.real;
			get<1>().get<1>() = q.dual;
		}
		auto inverse_view_transform() { return glm::dualquat{ get<1>().get<0>(), get<1>().get<1>() }; }

		auto& eye_position() { return get<2>(); }
	};
	struct proj_data : gl::std430<glm::vec4, glm::u32vec2, float, float> {
		auto& proj_xywz() { return get<0>(); }
		auto& backbuffer_size() { return get<1>(); }
		auto& tan_half_fovy() { return get<2>(); }
		auto& aspect() { return get<3>(); }
	};

	using view_buffer_type = gl::array<view_data>;
	using proj_buffer_type = gl::array<proj_data>;

private:
	view_buffer_type view_buffer;
	proj_buffer_type proj_buffer;

public:
	renderer_transform_buffers(const ste_context &ctx)
		: view_buffer(ctx, 1, gl::buffer_usage::storage_buffer),
		proj_buffer(ctx, 1, gl::buffer_usage::storage_buffer)
	{}

	void update_view_data(gl::command_recorder &recorder,
						  const primary_renderer_camera &c) {
		auto p = c.get_position();

		view_data v;
		v.set_view_transform(c.view_transform_dquat());
		v.inverse_view_transform() = glm::inverse(v.view_transform());
		v.eye_position() = glm::vec4{ p.x, p.y, p.z, .0f };

		recorder << view_buffer.overwrite_cmd(0, v);
	}

	void update_proj_data(gl::command_recorder &recorder,
						  const primary_renderer_camera &c,
						  const glm::uvec2 backbuffer_size) {
		proj_data p;

		p.proj_xywz() = c.get_projection_model().projection_xywz();
		p.backbuffer_size() = backbuffer_size;
		p.tan_half_fovy() = c.get_projection_model().tan_fovy_over_two();
		p.aspect() = c.get_projection_model().get_aspect();

		recorder << proj_buffer.overwrite_cmd(0, p);
	}

	auto& get_view_buffer() const { return view_buffer; }
	auto& get_proj_buffer() const { return proj_buffer; }
};

}
}
