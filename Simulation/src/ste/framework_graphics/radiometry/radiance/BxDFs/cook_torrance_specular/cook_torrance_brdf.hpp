// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <BxDF.hpp>

namespace ste {
namespace Graphics {

template <typename T,
		  template <typename> class NDF,
		  template <typename> class Fresnel,
		  template <typename> class GAF>
class cook_torrance_brdf : public BxDF<T> {
public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
				 const glm::tvec3<T> &v,
				 const glm::tvec3<T> &n,
				 const T &roughness,
				 const glm::tvec3<T> &cspec) {
		auto h = glm::normalize(v + l);

		auto d = NDF<T>()(h, n, roughness);
		auto g = GAF<T>()(l, v, n, h, roughness);
		auto f = Fresnel<T>()(l, h, cspec);

		auto denom = static_cast<T>(4) * glm::dot(n,l) * glm::dot(n,v);

		return d * g * f / denom;
	}
};

}
}
