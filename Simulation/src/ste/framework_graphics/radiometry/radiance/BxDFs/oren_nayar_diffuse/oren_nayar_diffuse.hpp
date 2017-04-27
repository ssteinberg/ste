// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <BxDF.hpp>

namespace ste {
namespace Graphics {

template <typename T>
class oren_nayar_brdf : public BxDF<T> {
public:
	T evaluate_r1(const T &roughness) const {
		auto roughness2 = roughness * roughness;
		return roughness2 / (roughness2 + static_cast<T>(0.33));
	}

	T evaluate_r2(const T &roughness) const {
		auto roughness2 = roughness * roughness;
		return static_cast<T>(0.45) * roughness2 / (roughness2 + static_cast<T>(0.09));
	}

	T evaluate_a(const T &dotNL, const T &roughness) const {
		auto r1 = evaluate_r1(roughness);
		return dotNL * (static_cast<T>(1) - r1 / static_cast<T>(2));
	}

public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
							 const glm::tvec3<T> &v,
							 const glm::tvec3<T> &n,
							 const T &roughness) {
		auto dotNL = glm::dot(n,l);
		auto dotVL = glm::dot(v,l);
		auto dotNV = glm::dot(n,v);

		auto r2 = evaluate_r2(roughness);

		auto s = glm::max<T>(0, dotVL - dotNV * dotNL);
		auto t = glm::min<T>(1, dotNL / dotNV);

		auto a = evaluate_a(dotNL, roughness);
		auto b = r2 * s * t;

		auto d = a + b;

		return d / glm::pi<T>();
	}
};

}
}
