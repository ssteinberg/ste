// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <BxDF.hpp>

namespace ste {
namespace Graphics {

template <typename T>
class lambert_diffuse_brdf : public BxDF<T> {
public:
	glm::tvec3<T> operator()() {
		return glm::one_over_pi<T>();
	}
};

}
}
