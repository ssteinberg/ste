// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace StE {
namespace Graphics {

template <typename T>
class ggx_ndf {
public:
	T evaluate(const T &dotNH, const T &roughness) const {
		auto alpha = roughness * roughness;
		auto alpha2 = alpha * alpha;
		auto dotNH2 = dotNH * dotNH;

		auto denom = dotNH2 * (alpha2 - static_cast<T>(1)) + 1;

		return alpha2 / (glm::pi<T>() * denom * denom);
	}

	T cdf(const T &dotNH, const T &roughness) const {
		auto alpha = roughness * roughness;
		auto alpha2 = alpha * alpha;
		auto dotNH2 = dotNH * dotNH;
		auto t = alpha2 - static_cast<T>(1);

		auto denom = t * (dotNH2 * t + 1);

		return alpha2 / denom - static_cast<T>(1) / t;
	}

public:
	T operator()(const glm::tvec3<T> &h, const glm::tvec3<T> &n, const T &roughness) const {
		auto dotNH = glm::dot(n,h);
		return evaluate(dotNH, roughness);
	}
};

}
}
