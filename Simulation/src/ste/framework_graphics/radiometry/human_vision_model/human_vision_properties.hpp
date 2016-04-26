// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Graphics {

class human_vision_properties {
public:
	static constexpr float min_luminance = .000001f;

public:
	static float scotopic_vision(float lum) {
		return 1.f - glm::clamp((lum - min_luminance) / (.1f - min_luminance), 0.f, 1.f);
	}

	static float mesopic_vision(float lum) {
		return 1.f - glm::clamp((lum - .01f) / (10.f - .01f), 0.f, 1.f);
	}

	static float visual_acuity(float lum) {
		float a = glm::mix(0.f, .7f, scotopic_vision(lum));
		return glm::pow(a, 4.f);
	}

	static float red_response(float lum) {
		return glm::mix(1.f, .35f, mesopic_vision(lum));
	}

	static float monochromaticity(float lum) {
		return glm::smoothstep(1.0f, .0f, glm::clamp((lum - min_luminance) / (.01f - min_luminance), 0.f, 1.f));
	}
};

}
}
