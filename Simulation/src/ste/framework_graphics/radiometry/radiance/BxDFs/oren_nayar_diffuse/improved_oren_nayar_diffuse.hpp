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
	T improved_oren_nayar_Lm(T r,
							 T dotNL) {
		T one_m_NdotL = 1 - dotNL;
		T one_m_NdotL2 = one_m_NdotL * one_m_NdotL;

		T a = glm::max(1 - 2 * r, .0f);
		T b = 1 - one_m_NdotL2 * one_m_NdotL2 * one_m_NdotL;
		T p1 = a * b + glm::min(2*r, 1);

		T p2 = dotNL * (1 - .5f * r) + .5f * r * dotNL * dotNL;

		return p1 * p2;
	}

	T improved_oren_nayar_Vd(T r2,
							 T dotNL,
							 T dotNV) {
		T power = (1 - 0.3726732f * dotNV * dotNV) / (.188566f + .388410 * dotNV);
		T p1 = r2 / ((r2 + .09f) * (1.31072f + .995584f * dotNV));
		T p2 = 1 - glm::pow(1 - dotNL, power);

		return p1 * p2;
	}

	T improved_oren_nayar_Bp(T dotNL,
							 T dotNV,
							 T dotVL) {
		T delta = dotVL - dotNV * dotNL;
		if (delta < .0f)
			return 1.4f * dotNV * dotNL * delta;
		else
			return delta;
	}

	T improved_oren_nayar_Fr(T r,
							 T r2,
							 T dotNV) {
		T r3 = r2 * r;
		T r4 = r2 * r2;

		T p1 = 1 - (.542026f * r2 + .303573 * r) / (r2 + 1.36053f);
		T p2 = 1 - glm::pow(1 - dotNV, 5 - 4 * r2);
		T p3 = (-.733996f*r3 + 1.50912*r2 - 1.16402*r) * glm::pow(1 - dotNV, 1 + 1 / (39 * r4 + 1)) + 1;

		return p1 * p2 * p3;
	}

public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
				 const glm::tvec3<T> &v,
				 const glm::tvec3<T> &n,
				 const T &roughness,
				 const glm::tvec3<T> &f0) {
		float roughness2 = roughness * roughness;
		float dotNL = glm::dot(n,l);
		float dotVL = glm::dot(v,l);
		float dotNV = glm::dot(n,v);

		float Lm = improved_oren_nayar_Lm(roughness, dotNL);
		float Vd = improved_oren_nayar_Vd(roughness2, dotNL, dotNV);
		float Bp = improved_oren_nayar_Bp(dotNL, dotNV, dotVL);
		float Fr = improved_oren_nayar_Fr(roughness, roughness2, dotNV);

		return 21.f / (20.f * glm::pi<T>()) * (glm::vec3(1) - f0) * (Fr * Lm + Vd * Bp);
	}
};

}
}
