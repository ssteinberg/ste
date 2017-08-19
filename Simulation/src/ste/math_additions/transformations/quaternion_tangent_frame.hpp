// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ste {

namespace _detail {

inline glm::mat2x3 extract_tangent_bitangent_from_tangent_frame(const glm::quat &q) {
	return glm::mat2x3{
		1.f - 2.f*(q.y*q.y + q.z*q.z),	2.f*(q.x*q.y + q.w*q.z),		2.f*(q.x*q.z - q.w*q.y),
		2.f*(q.x*q.y - q.w*q.z),		1.f - 2.f*(q.x*q.x + q.z*q.z),	2.f*(q.y*q.z + q.w*q.x)
	};
}

}

inline glm::quat tbn_to_tangent_frame(const glm::vec3 &t, const glm::vec3 &b, const glm::vec3 &n) {
	glm::mat3 rotation{ b, t, n };
	glm::quat tangent_frame_quat(rotation);

	float det = glm::determinant(rotation);
	float reflection = det >= .0f ? 1.f : -1.f;

	// w represents reflection => can't encode w as zero
	constexpr float delta = 1e-8f;
	if (glm::abs(tangent_frame_quat.w) < delta) {
		float renorm = glm::sqrt(1.f - delta * delta);
		tangent_frame_quat.x *= renorm;
		tangent_frame_quat.y *= renorm;
		tangent_frame_quat.z *= renorm;
		tangent_frame_quat.w = (2.f * glm::step(.0f, tangent_frame_quat.w) - 1.f) * delta;
	}

	if (tangent_frame_quat.w < .0f)
		tangent_frame_quat = -tangent_frame_quat;
	tangent_frame_quat *= reflection;

	return tangent_frame_quat;
}

inline glm::mat3x3 extract_tangent_frame(const glm::quat &transform, const glm::quat &tangent_frame) {
	glm::quat q = transform * tangent_frame;
	glm::mat2x3 bt = _detail::extract_tangent_bitangent_from_tangent_frame(q);

	return glm::mat3x3{
		bt[1],
		bt[0],
		glm::cross(bt[0],bt[1]) * glm::sign(tangent_frame.w)
	};
}

}
