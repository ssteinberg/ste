// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "BxDF.hpp"

namespace StE {
namespace Graphics {

template <typename T,
		  template <typename> class NDF,
		  template <typename> class F,
		  template <typename> class G>
class cook_torrance_brdf : public BxDF<T> {
public:
	T operator()(const glm::tvec3<T> &l,
				 const glm::tvec3<T> &v,
				 const glm::tvec3<T> &n,
				 const T &roughness,
				 const T &cspec) {
		auto h = glm::normalize(v + l);

		auto d = NDF<T>()(h, n, roughness);
		auto g = G<T>()(l, v, n, roughness);
		auto f = F<T>()(l, h, cspec);

		auto denom = static_cast<T>(4) * glm::dot(n,l) * glm::dot(n,v);

		return d * g * f / denom;
	}
};

}
}
