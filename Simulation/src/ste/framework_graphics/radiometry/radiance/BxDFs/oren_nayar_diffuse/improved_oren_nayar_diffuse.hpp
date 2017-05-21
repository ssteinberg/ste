// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <BxDF.hpp>

namespace ste {
namespace graphics {

template <typename T>
class oren_nayar_brdf : public BxDF<T> {
public:
	T improved_oren_nayar_Lm(T r,
							 T dotNL) {
		T one_m_NdotL = static_cast<T>(1) - dotNL;
		T one_m_NdotL2 = one_m_NdotL * one_m_NdotL;

		T a = glm::max(static_cast<T>(1) - static_cast<T>(2) * r, static_cast<T>(.0));
		T b = static_cast<T>(1) - one_m_NdotL2 * one_m_NdotL2 * one_m_NdotL;
		T p1 = a * b + glm::min(static_cast<T>(2)*r, static_cast<T>(1));

		T p2 = dotNL * (static_cast<T>(1) - static_cast<T>(.5) * r) + static_cast<T>(.5) * r * dotNL * dotNL;

		return p1 * p2;
	}

	T improved_oren_nayar_Vd(T r2,
							 T dotNL,
							 T dotNV) {
		T power = (static_cast<T>(1) - static_cast<T>(0.3726732) * dotNV * dotNV) / (static_cast<T>(.188566) + static_cast<T>(.388410) * dotNV);
		T p1 = r2 / ((r2 + static_cast<T>(.09)) * (static_cast<T>(1.31072) + static_cast<T>(.995584) * dotNV));
		T p2 = static_cast<T>(1) - glm::pow(1 - dotNL, power);

		return p1 * p2;
	}

	T improved_oren_nayar_Bp(T dotNL,
							 T dotNV,
							 T dotVL) {
		T delta = dotVL - dotNV * dotNL;
		if (delta < static_cast<T>(.0))
			return static_cast<T>(1.4) * dotNV * dotNL * delta;
		else
			return delta;
	}

	T improved_oren_nayar_Fr(T r,
							 T r2,
							 T dotNV) {
		T r3 = r2 * r;
		T r4 = r2 * r2;

		T p1 = static_cast<T>(1) - (static_cast<T>(.542026) * r2 + static_cast<T>(.303573) * r) / (r2 + static_cast<T>(1.36053));
		T p2 = static_cast<T>(1) - glm::pow(1 - dotNV, 5 - 4 * r2);
		T p3 = (-static_cast<T>(.733996)*r3 + static_cast<T>(1.50912)*r2 - static_cast<T>(1.16402)*r) * glm::pow(static_cast<T>(1) - dotNV, static_cast<T>(1) + static_cast<T>(1) / (static_cast<T>(39) * r4 + static_cast<T>(1))) + static_cast<T>(1);

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
