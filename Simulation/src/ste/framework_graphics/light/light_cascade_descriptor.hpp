// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <observable_resource.hpp>

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

struct light_cascade_data {
	glm::mat3x4 M;
	glm::vec2 recp_vp;
	float proj_eye_dist;
	float proj_far_clip;

	static auto generate(const glm::vec3 &X, const glm::vec3 &Y,
						 const glm::vec3 &l,
						 float cascade_eye_dist,
						 const glm::vec2 &recp_viewport,
						 float near_clip,
						 float far_clip) {
		glm::vec3 center = { 0, 0, -glm::mix(near_clip, far_clip, .5f) };
		glm::vec3 eye = center - l * cascade_eye_dist;

		glm::vec3 sX = X * recp_viewport.x;
		glm::vec3 sY = Y * recp_viewport.y;

		glm::mat3x4 M = glm::mat3x4(
			glm::vec4(sX, -glm::dot(eye, sX)),
			glm::vec4(sY, -glm::dot(eye, sY)),
			glm::vec4(-l, glm::dot(eye, l))
		);

		return M;
	}
};

struct light_cascades_descriptor {
	light_cascade_data cascades[directional_light_cascades];
};

}
}
