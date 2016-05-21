// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "BxDF.hpp"

namespace StE {
namespace Graphics {

template <typename T>
class disney_diffuse_brdf : public BxDF<T> {
public:
	glm::tvec3<T> operator()(const glm::tvec3<T> &l,
							 const glm::tvec3<T> &v,
							 const glm::tvec3<T> &n,
							 const T &roughness) {
	}
};

}
}
