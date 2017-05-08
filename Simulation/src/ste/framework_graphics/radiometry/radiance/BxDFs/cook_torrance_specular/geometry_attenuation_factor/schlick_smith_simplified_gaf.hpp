// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

template <typename T>
class schlick_smith_simplified_gaf {
public:
	T evaluate(const T &dotLH, const T &roughness) const {
		auto k = roughness * roughness / static_cast<T>(2);
		auto k2 = k * k;
		auto t = static_cast<T>(1) - k2;
		auto g = dotLH * dotLH * t + k2;

		return static_cast<T>(1) / g;
	}

	T operator()(const glm::tvec3<T> &l, const glm::tvec3<T> &v, const glm::tvec3<T> &n, const glm::tvec3<T> &h, const T &roughness) const {
		auto dotLH = glm::dot(l,h);
		return evaluate(dotLH, roughness);
	}
};

}
}
