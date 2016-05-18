// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Graphics {

template <typename T>
class schlick_fresnel {
public:
	T operator()(const glm::tvec3<T> &l, const glm::tvec3<T> &h, const T &cspec) const {
		auto p = static_cast<T>(1) - glm::dot(l,h);
		auto p2 = p*p;
		auto p4 = p2*p2;
		auto p5 = p4*p;
		return cspec + (static_cast<T>(1) - cspec) * p5;
	}
};

}
}
