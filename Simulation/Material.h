// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Texture2D.h"
#include "BRDF.h"

#include <memory>

namespace StE {
namespace Graphics {

class Material {
protected:
	std::shared_ptr<LLR::Texture2D> diffuse;
	std::shared_ptr<LLR::Texture2D> specular;
	std::shared_ptr<LLR::Texture2D> normalmap;
	std::shared_ptr<LLR::Texture2D> alphamap;

	std::shared_ptr<BRDF> brdf;

	float emission{ 0 };

public:
	void set_diffuse(const std::shared_ptr<LLR::Texture2D> &tex) { diffuse = tex; }
	void set_specular(const std::shared_ptr<LLR::Texture2D> &tex) { specular = tex; }
	void set_normalmap(const std::shared_ptr<LLR::Texture2D> &tex) { normalmap = tex; }
	void set_alphamap(const std::shared_ptr<LLR::Texture2D> &tex) { alphamap = tex; }

	void set_emission(float t) { emission = t; }

	void set_brdf(const std::shared_ptr<BRDF> &b) { brdf = b; }

	const LLR::Texture2D *get_diffuse() const { return diffuse.get(); }
	const LLR::Texture2D *get_specular() const { return specular.get(); }
	const LLR::Texture2D *get_normalmap() const { return normalmap.get(); }
	const LLR::Texture2D *get_alphamap() const { return alphamap.get(); }

	float get_emission() const { return emission; }

	const BRDF *get_brdf() const { return brdf.get(); }
};

}
}
