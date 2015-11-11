// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "BxDF.h"

#include "common_brdf_representation.h"

#include "Texture2DArray.h"
#include "Sampler.h"

#include <memory>
#include <vector>

namespace StE {
namespace Graphics {

class BRDF : public BxDF {
public:
	struct brdf_descriptor {
		LLR::texture_handle tex_handler;
		int min_theta_in, max_theta_in;
	};

	static constexpr int theta_min = 0;
	static constexpr int theta_max = 90;
	static constexpr int phi_min = -180;
	static constexpr int phi_max = 180;

private:
	int min_theta, max_theta;
	std::unique_ptr<LLR::Texture2DArray> texture;

public:
	BRDF(const common_brdf_representation &brdf) : min_theta(glm::radians(brdf.get_min_incident())), max_theta(glm::radians(brdf.get_max_incident())) {
		texture = std::make_unique<LLR::Texture2DArray>(brdf.get_data(), true);
	}

	brdf_descriptor descriptor() const {
		brdf_descriptor desc;
		desc.max_theta_in = max_theta;
		desc.min_theta_in = min_theta;
		desc.tex_handler = texture->get_texture_handle();

		return desc;
	}

	int min_incident_theta() const { return min_theta; }
	int max_incident_theta() const { return max_theta; }
	const auto *brdf_texture() const { return texture.get(); }
};

}
}
