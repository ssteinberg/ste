// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "BxDF.hpp"

#include "common_brdf_representation.hpp"

#include "Texture3D.hpp"
#include "Sampler.hpp"

#include <memory>
#include <vector>

namespace StE {
namespace Graphics {

class BRDF : public BxDF {
public:
	struct brdf_descriptor {
		LLR::texture_handle tex_handler;
		std::int32_t min_theta_in, max_theta_in;
	};

	static constexpr int theta_min = 0;
	static constexpr int theta_max = 90;
	static constexpr int phi_min = -179;
	static constexpr int phi_max = 180;

private:
	int min_theta, max_theta;
	std::unique_ptr<LLR::Texture3D> texture;
	LLR::SamplerMipmapped sampler;

public:
	BRDF(const common_brdf_representation &brdf) : 	min_theta(glm::radians(static_cast<float>(brdf.get_min_incident()))), 
													max_theta(glm::radians(static_cast<float>(brdf.get_max_incident()))), 
													sampler(LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear) {
		texture = std::make_unique<LLR::Texture3D>(*brdf.get_data(), true);
		sampler.set_wrap_r(LLR::TextureWrapMode::Wrap);
		sampler.set_wrap_s(LLR::TextureWrapMode::ClampToEdge);
		sampler.set_wrap_t(LLR::TextureWrapMode::ClampToEdge);
	}

	brdf_descriptor descriptor() const {
		brdf_descriptor desc;
		desc.max_theta_in = max_theta;
		desc.min_theta_in = min_theta;
		desc.tex_handler = texture->get_texture_handle(sampler);

		return desc;
	}

	int min_incident_theta() const { return min_theta; }
	int max_incident_theta() const { return max_theta; }
	const auto *brdf_texture() const { return texture.get(); }
};

}
}
