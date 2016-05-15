// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include <glm/gtc/quaternion.hpp>

namespace StE {

namespace _detail {

glm::mat2x3 extract_tangent_bitangent_from_tangent_frame(const glm::quat &q) {
	return glm::mat2x3{
		1.f - 2.f*(q.y*q.y + q.z*q.z),	2.f*(q.x*q.y + q.w*q.z),		2.f*(q.x*q.z - q.w*q.y),
		2.f*(q.x*q.y - q.w*q.z),		1.f - 2.f*(q.x*q.x + q.z*q.z),	2.f*(q.y*q.z + q.w*q.x)
	};
}

}

glm::quat tbn_to_tangent_frame(const glm::vec3 &t, const glm::vec3 &b, const glm::vec3 &n) {
	glm::mat3 rotation{ t, b, n };
	float det = glm::determinant(rotation);

	float scale = det > .0f ? -1.f : 1.f;
	rotation[0].z *= scale;
	rotation[1].z *= scale;
	rotation[2].z *= scale;

	glm::quat tangent_frame_quat = glm::quat_cast(rotation);

	// w represents reflection => can't encode w as zero
	constexpr float delta = .0000001f;
	if (glm::abs(tangent_frame_quat.w) < delta) {
		float renorm = glm::sqrt(1.f - delta * delta);
		tangent_frame_quat.x *= renorm;
		tangent_frame_quat.y *= renorm;
		tangent_frame_quat.z *= renorm;
		tangent_frame_quat.w = tangent_frame_quat.w > .0f ? delta : -delta;
	}

	float sign = glm::sign(scale * tangent_frame_quat.w);
	tangent_frame_quat.x *= sign;
	tangent_frame_quat.y *= sign;
	tangent_frame_quat.z *= sign;
	tangent_frame_quat.w *= sign;

	return tangent_frame_quat;
}

glm::mat3x3 extract_tangent_frame(const glm::quat &transform, const glm::quat &tangent_frame) {
	glm::quat q = transform * tangent_frame;
	glm::mat2x3 tb = _detail::extract_tangent_bitangent_from_tangent_frame(q);

	return glm::mat3x3{
		tb[0],
		tb[1],
		glm::cross(tb[0],tb[1]) * glm::sign(tangent_frame.w)
	};
}

}
