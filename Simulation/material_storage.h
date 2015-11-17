// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "Material.h"

#include "texture_handle.h"
#include "Sampler.h"
#include "BRDF.h"

#include "gstack.h"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

class material_storage {
private:
	struct material_texture_descriptor {
		LLR::texture_handle tex_handler;
	};
	struct material_descriptor {
		material_texture_descriptor diffuse;
		material_texture_descriptor specular;
		material_texture_descriptor normalmap;
		material_texture_descriptor alphamap;
		BRDF::brdf_descriptor brdf;
		glm::vec4 emission;
	};

	std::vector<std::shared_ptr<Material>> materials;
	LLR::SamplerMipmapped linear_sampler;
	LLR::gstack<material_descriptor> stack;

public:
	material_storage() {
		linear_sampler.set_wrap_s(LLR::TextureWrapMode::Wrap);
		linear_sampler.set_wrap_t(LLR::TextureWrapMode::Wrap);
		linear_sampler.set_wrap_r(LLR::TextureWrapMode::Wrap);
		linear_sampler.set_min_filter(LLR::TextureFiltering::Linear);
		linear_sampler.set_mag_filter(LLR::TextureFiltering::Linear);
		linear_sampler.set_mipmap_filter(LLR::TextureFiltering::Linear);
		linear_sampler.set_anisotropic_filter(16);
	}

	std::size_t add_material(const std::shared_ptr<Material> &material) {
		auto diffuse = material->get_diffuse();
		auto specular = material->get_specular();
		auto normalmap = material->get_normalmap();
		auto alphamap = material->get_alphamap();
		auto brdf = material->get_brdf();

		material_descriptor md;
		if (diffuse != nullptr) {
			md.diffuse.tex_handler = diffuse->get_texture_handle(linear_sampler);
			md.diffuse.tex_handler.make_resident();
		}
		if (specular != nullptr) {
			md.specular.tex_handler = specular->get_texture_handle(linear_sampler);
			md.specular.tex_handler.make_resident();
		}
		if (normalmap != nullptr) {
			md.normalmap.tex_handler = normalmap->get_texture_handle(linear_sampler);
			md.normalmap.tex_handler.make_resident();
		}
		if (alphamap != nullptr) {
			md.alphamap.tex_handler = alphamap->get_texture_handle(linear_sampler);
			md.alphamap.tex_handler.make_resident();
		}
		if (brdf != nullptr) {
			md.brdf = brdf->descriptor();
			md.brdf.tex_handler.make_resident();
		}
		md.emission.xyz = material->get_emission();

		materials.push_back(material);
		stack.push_back(md);

		return stack.size() - 1;
	}

	std::size_t add_materials(const std::vector<std::shared_ptr<Material>> &mats) {
		int i = stack.size();
		for (auto &m : mats)
			add_material(m);
		return i;
	}

	auto &buffer() const { return stack.get_buffer(); }
};

}
}
