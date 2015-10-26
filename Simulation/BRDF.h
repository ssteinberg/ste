// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "BxDF.h"

#include "Texture3D.h"
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
	std::unique_ptr<LLR::Texture3D> texture;

public:
	BRDF(int min_theta_in, int max_theta_in, const gli::texture3D &data_tex) : min_theta(min_theta_in), max_theta(max_theta_in) {
		texture = std::make_unique<LLR::Texture3D>(data_tex, true);
	}

	brdf_descriptor descriptor() const {
		LLR::SamplerMipmapped sam(LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear, LLR::TextureFiltering::Linear);
		sam.set_wrap_s(LLR::TextureWrapMode::Wrap);
		sam.set_wrap_t(LLR::TextureWrapMode::ClampToEdge);
		sam.set_wrap_r(LLR::TextureWrapMode::ClampToEdge);

		brdf_descriptor desc;
		desc.max_theta_in = max_theta;
		desc.min_theta_in = min_theta;
		desc.tex_handler = texture->get_texture_handle(sam);

		return desc;
	}

	int min_incident_theta() const { return min_theta; }
	int max_incident_theta() const { return max_theta; }
	const LLR::Texture3D *brdf_texture() const { return texture.get(); }
};

}
}
