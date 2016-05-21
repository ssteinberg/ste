// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "Material.hpp"

#include "texture_handle.hpp"
#include "Sampler.hpp"
#include "pBRDF.hpp"

#include "gstack.hpp"

#include <vector>
#include <memory>

namespace StE {
namespace Graphics {

class material_storage {
private:
	struct material_texture_descriptor {
		Core::texture_handle tex_handler;
	};
	struct material_descriptor {
		material_texture_descriptor basecolor_map;
		material_texture_descriptor cavity_map;
		material_texture_descriptor normal_map;
		material_texture_descriptor mask_map;

		glm::vec3 emission;
		float roughness;
		float anisotropy_ratio;
		float metallic;
		float F0;
		float sheen;
	};

	std::vector<std::shared_ptr<Material>> materials;
	Core::SamplerMipmapped linear_sampler;
	Core::gstack<material_descriptor> stack;

public:
	material_storage() {
		linear_sampler.set_wrap_s(Core::TextureWrapMode::Wrap);
		linear_sampler.set_wrap_t(Core::TextureWrapMode::Wrap);
		linear_sampler.set_wrap_r(Core::TextureWrapMode::Wrap);
		linear_sampler.set_min_filter(Core::TextureFiltering::Linear);
		linear_sampler.set_mag_filter(Core::TextureFiltering::Linear);
		linear_sampler.set_mipmap_filter(Core::TextureFiltering::Linear);
		linear_sampler.set_anisotropic_filter(8);
	}

	std::size_t add_material(const std::shared_ptr<Material> &material) {
		auto basecolor_map = material->get_basecolor_map();
		auto cavity_map = material->get_cavity_map();
		auto normal_map = material->get_normal_map();
		auto mask_map = material->get_mask_map();

		material_descriptor md;
		if (basecolor_map != nullptr) {
			md.basecolor_map.tex_handler = basecolor_map->get_texture_handle(linear_sampler);
			md.basecolor_map.tex_handler.make_resident();
		}
		if (cavity_map != nullptr) {
			md.cavity_map.tex_handler = cavity_map->get_texture_handle(linear_sampler);
			md.cavity_map.tex_handler.make_resident();
		}
		if (normal_map != nullptr) {
			md.normal_map.tex_handler = normal_map->get_texture_handle(linear_sampler);
			md.normal_map.tex_handler.make_resident();
		}
		if (mask_map != nullptr) {
			md.mask_map.tex_handler = mask_map->get_texture_handle(linear_sampler);
			md.mask_map.tex_handler.make_resident();
		}
		md.emission = material->get_emission();
		md.roughness = material->get_roughness();
		md.anisotropy_ratio = material->get_anisotropy_ratio_from_anisotropy();
		md.metallic = material->get_metallic();
		md.F0 = material->get_F0_from_ior();
		md.sheen = material->get_sheen();

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
