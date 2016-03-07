// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace Graphics {

class BxDF {
public:
	static float cos_theta(const glm::vec3 &v) { return v.z; }
	static float sin2_theta(const glm::vec3 &v) { return glm::max(.0f, 1.f - cos_theta(v)*cos_theta(v)); }
	static float sin_theta(const glm::vec3 &v) { return glm::sqrt(sin2_theta(v)); }
	static float cos_phi(const glm::vec3 &v) {
		float s = sin_theta(v);
		if (s == .0f) return 1.f;
		return glm::clamp(v.x / s, -1.f, 1.f);
	}
	static float sin_phi(const glm::vec3 &v) {
		float s = sin_theta(v);
		if (s == .0f) return 0.f;
		return glm::clamp(v.y / s, -1.f, 1.f);
	}
	static glm::vec3 omega(float theta, float phi) {
		auto z = glm::cos(glm::radians(theta));
		auto x = glm::cos(glm::radians(phi));
		auto y = glm::sin(glm::radians(phi));

		glm::vec2 xy(x, y);
		xy *= glm::sqrt(1.f - z*z);

		return{ xy.x, xy.y, z };
	}
};

}
}
