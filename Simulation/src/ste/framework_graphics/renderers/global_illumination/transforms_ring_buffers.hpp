// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "camera.hpp"

#include "ring_buffer.hpp"
#include "range.hpp"

#include <glm/gtx/dual_quaternion.hpp>

namespace StE {
namespace Graphics {

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

	using view_ring_buffer_type = Core::ring_buffer<view_data, 3>;
	using proj_ring_buffer_type = Core::ring_buffer<proj_data, 1>;

private:
	view_ring_buffer_type view_buffer;
	proj_ring_buffer_type proj_buffer;
	range<> r{ 0, 0 };

public:
	void update_view_data(const camera &c) {
		auto p = c.get_position();

		view_data v;
		v.view_transform = c.view_transform_dquat();
		v.inverse_view_transform = glm::inverse(v.view_transform);
		v.eye_position = glm::vec4{ p.x, p.y, p.z, .0f };

		range<> r = view_buffer.commit(v);
		r.start /= sizeof(view_data);
		r.length /= sizeof(view_data);

		this->r = r;
	}

	void update_proj_data(float fovy, float aspect, float fnear, const glm::uvec2 backbuffer_size) {
		proj_data p;

		float tanHalfFovy = glm::tan(fovy * .5f);
		float one_over_tan_half_fovy = 1.f / tanHalfFovy;
		float one_over_aspect_tan_half_fovy = 1.f / (aspect * tanHalfFovy);

		p.proj_xywz = { one_over_aspect_tan_half_fovy, one_over_tan_half_fovy, fnear, -1.f };
		p.backbuffer_size = backbuffer_size;
		p.tan_half_fovy = tanHalfFovy;
		p.aspect = aspect;

		proj_buffer.commit(p);
	}

	void bind_view_buffer(int idx) const {
		view_buffer.get_buffer().bind_range(Core::shader_storage_layout_binding(idx), r.start, r.length);
	}

	void bind_proj_buffer(int idx) const {
		proj_buffer.get_buffer().bind_range(Core::shader_storage_layout_binding(idx), 0, 1);
	}
};

}
}
