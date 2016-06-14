// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "texture_handle.hpp"

namespace StE {
namespace Graphics {

static constexpr std::uint32_t material_layer_none = 0xFFFFFFFF;
static constexpr float material_layer_max_thickness = .1f;

class material_layer_descriptor {
public:
	Core::texture_handle basecolor_handle;
	
private:
	std::uint32_t sheen_pack;
	std::uint32_t ansi_metal_pack;
	std::uint32_t roughness_thickness_pack;
	
public:
	float ior{ 1.5f };
	float absorption_alpha{ .0f };

	std::uint32_t next_layer_id{ material_layer_none };

public:
	material_layer_descriptor() : sheen_pack(glm::packUnorm2x16({.0f, .0f})),
								  ansi_metal_pack(glm::packUnorm2x16({.5f, .0f})),
								  roughness_thickness_pack(glm::packUnorm2x16({.5f, .0f})) {}
	
	material_layer_descriptor(material_layer_descriptor &&) = default;
	material_layer_descriptor(const material_layer_descriptor &) = default;
	material_layer_descriptor &operator=(material_layer_descriptor &&) = default;
	material_layer_descriptor &operator=(const material_layer_descriptor &) = default;
	
	void set_sheen(float sheen, float sheen_power) {
		sheen = glm::clamp(sheen, .0f, 1.f);
		sheen_power = glm::clamp(sheen_power, .0f, 1.f);
		sheen_pack = glm::packUnorm2x16({ sheen, sheen_power });
	}
	
	void set_anisotropy_and_metallicity(float ansi_ratio, float metallic) {
		ansi_ratio = glm::clamp(ansi_ratio, -1.f, 1.f);
		metallic = glm::clamp(metallic, .0f, 1.f);
		ansi_metal_pack = glm::packUnorm2x16({ ansi_ratio * .5f + .5f, metallic });
	}
	
	void set_roughness_and_thickness(float roughness, float thickness) {
		roughness = glm::clamp(roughness, .0f, 1.f);
		thickness = glm::clamp(thickness, .0f, material_layer_max_thickness);
		roughness_thickness_pack = glm::packUnorm2x16({ roughness, thickness / material_layer_max_thickness });
	}
};

}
}
