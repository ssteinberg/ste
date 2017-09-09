//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

#include <glm/gtc/quaternion.hpp>

namespace ste {
namespace graphics {

struct mesh_descriptor : gl::std430<glm::mat3x4, glm::quat, glm::vec4, std::int32_t, std::int32_t, float, float> {
	auto& model_transform_matrix() { return get<0>(); }
	auto& tangent_transform_quat() { return get<1>(); }

	auto& bounding_sphere() { return get<2>(); }

	auto& mat_idx() { return get<3>(); }
	auto& light_caster() { return get<4>(); }
};

struct mesh_draw_params : gl::std430<std::uint32_t, std::uint32_t, std::int32_t> {
	auto& count() { return get<0>(); }
	auto& first_index() { return get<1>(); }
	auto& vertex_offset() { return get<2>(); }
};

}
}
