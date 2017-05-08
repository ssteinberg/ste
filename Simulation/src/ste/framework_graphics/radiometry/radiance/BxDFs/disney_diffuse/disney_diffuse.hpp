// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <BxDF.hpp>

#include <schlick_fresnel.hpp>

namespace ste {
namespace graphics {

template <typename T>
class disney_diffuse_brdf : public BxDF<T> {
public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
							 const glm::tvec3<T> &v,
							 const glm::tvec3<T> &n,
							 const T &roughness) {
		T dotLH = glm::dot(l,h);

		T fresnel_l = schlick_fresnel<T>().fresnel_k(glm::dot(n,l));
		T fresnel_v = schlick_fresnel<T>().fresnel_k(glm::dot(n,v));
		T Fd90 = static_cast<T>(.5) + static_cast<T>(2) * dotLH*dotLH * roughness;
		return glm::mix(static_cast<T>(1), Fd90, fresnel_l) * glm::mix(static_cast<T>(1), Fd90, fresnel_v) * glm::one_over_pi<T>();
	}
};

}
}
