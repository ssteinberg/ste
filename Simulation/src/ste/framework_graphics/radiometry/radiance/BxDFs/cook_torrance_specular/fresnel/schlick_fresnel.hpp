// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace Graphics {

template <typename T>
class schlick_fresnel {
public:
	static float fresnel_k(float d) {
		auto p = static_cast<T>(1) - d;
		auto p2 = p*p;
		return p * p2 * p2;
	}

public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l, const glm::tvec3<T> &h, const glm::tvec3<T> &cspec) const {
		T d = glm::dot(l,h);
		return cspec + (glm::tvec3<T>(1) - cspec) * fresnel_k(d);
	}
};

}
}
