// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "BxDF.hpp"

namespace StE {
namespace Graphics {

template <typename T>
class oren_nayar_brdf : public BxDF<T> {
public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
				 const glm::tvec3<T> &v,
				 const glm::tvec3<T> &n,
				 const T &roughness,
				 const glm::tvec3<T> &albedo,
				 const glm::tvec3<T> &irradiance) {
		auto roughness2 = roughness * roughness;
		auto dotNL = glm::dot(n,l);
		auto dotVL = glm::dot(v,l);
		auto dotNV = glm::dot(n,v);

		auto r1 = roughness2 / (roughness2 + static_cast<T>(0.33));
		auto r2 = static_cast<T>(0.45) * roughness2 / (roughness2 + static_cast<T>(0.09));

		auto s = glm::max<T>(0, dotVL - dotNV * dotNL);
		auto t = glm::min<T>(1, dotNL / dotNV);

		auto a = dotNL * (static_cast<T>(1) - r1 / static_cast<T>(2));
		auto b = r2 * s * t;

		auto d = a + b;

		return albedo * irradiance * d / glm::pi<T>();
	}
};

}
}
