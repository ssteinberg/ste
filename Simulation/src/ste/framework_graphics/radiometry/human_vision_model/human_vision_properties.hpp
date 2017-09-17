// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

class human_vision_properties {
public:
	static constexpr float min_luminance = 1e-9f;

	static constexpr float scotopic_end = 3e-3f;
	static constexpr float mesopic_end = 3.f;

	static constexpr float visual_acuity_coef = .9f;

public:
	static float scotopic_vision(float lum) {
		return 1.f - glm::clamp((lum - min_luminance) / (scotopic_end - min_luminance), 0.f, 1.f);
	}

	static float mesopic_vision(float lum) {
		return 1.f - glm::clamp((lum - scotopic_end) / (mesopic_end - scotopic_end), 0.f, 1.f);
	}

	static float visual_acuity(float lum) {
		const float a = glm::mix(0.f, visual_acuity_coef, mesopic_vision(lum));
		return a*a;
	}

	static float red_response(float lum) {
		return glm::mix(1.f, .35f, scotopic_vision(lum));
	}

	static float monochromaticity(float lum) {
		return glm::smoothstep(.0f, 1.f, mesopic_vision(lum));
	}
};

}
}
