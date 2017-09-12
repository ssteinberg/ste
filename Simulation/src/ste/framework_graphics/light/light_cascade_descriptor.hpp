// StE
// � Shlomi Steinberg, 2015-201
#pragma once

#include <stdafx.hpp>

#include <std430.hpp>

namespace ste {
namespace graphics {

constexpr std::size_t directional_light_cascades = 6;

constexpr float cascade_viewport_reserve = 1.025f;

// Select a cascade eye distance that gives enough range from the near-clip to the view frustum encompassed by the cascade
// to capture medium distance occluders, while maintaining depth precision. Keep it constant amongst cascades to avoid shadow filtering
// artifacts when moving between cascades.
// The near clip distance also reflects this motivation.
constexpr float cascade_projection_eye_distance = 5000.f;
constexpr float cascade_projection_near_clip = 1000.f;

struct light_cascade_data : gl::std430<glm::mat3x4, glm::vec2, float, float> {
	auto& M() { return get<0>(); }
	auto& recp_vp() { return get<1>(); }
	auto& proj_eye_dist() { return get<2>(); }
	auto& proj_far_clip() { return get<3>(); }

	auto& M() const { return get<0>(); }
	auto& recp_vp() const { return get<1>(); }
	auto& proj_eye_dist() const { return get<2>(); }
	auto& proj_far_clip() const { return get<3>(); }

	/*
	 *	@brief	Generates a cascade transform matrix
	 */
	static auto generate(const glm::vec3 &X, const glm::vec3 &Y,
						 const glm::vec3 &l,
						 float cascade_eye_dist,
						 const glm::vec2 &recp_viewport,
						 float near_clip,
						 float far_clip) {
		const glm::vec3 center = { 0, 0, -glm::mix(near_clip, far_clip, .5f) };
		const glm::vec3 eye = center - l * cascade_eye_dist;

		const glm::vec3 sX = X * recp_viewport.x;
		const glm::vec3 sY = Y * recp_viewport.y;

		const glm::mat3x4 M = glm::mat3x4(
			glm::vec4(sX, -glm::dot(eye, sX)),
			glm::vec4(sY, -glm::dot(eye, sY)),
			glm::vec4(-l,  glm::dot(eye, l))
		);

		return M;
	}
};

struct light_cascades_descriptor : gl::std430<light_cascade_data[directional_light_cascades]> {
	auto& cascades() { return get<0>(); }
	auto& cascades() const { return get<0>(); }
};

}
}
