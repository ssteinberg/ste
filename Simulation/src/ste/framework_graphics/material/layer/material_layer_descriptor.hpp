// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "texture_handle.hpp"

namespace StE {
namespace Graphics {

static constexpr std::uint32_t material_layer_none = 0xFFFFFFFF;
static constexpr float material_layer_max_thickness = .1f;
static constexpr float material_layer_ansio_ratio_scale = .9f;
extern const float material_layer_max_ansio_ratio;
extern const float material_layer_min_ansio_ratio;

class material_layer_descriptor {
public:
	Core::texture_handle basecolor_handle;
	
private:
	std::uint32_t sheen_pack{ 0 };
	std::uint32_t ansi_metal_pack{ 0 };
	std::uint32_t roughness_thickness_pack{ 0 };
	
	float ior{ .0f };
	float attenuation_coefficient{ .0f };

	std::uint32_t next_layer_id{ material_layer_none };

public:
	material_layer_descriptor() = default;
	
	material_layer_descriptor(material_layer_descriptor &&) = default;
	material_layer_descriptor(const material_layer_descriptor &) = default;
	material_layer_descriptor &operator=(material_layer_descriptor &&) = default;
	material_layer_descriptor &operator=(const material_layer_descriptor &) = default;
	
	void set_sheen(float sheen, float sheen_power) {
		sheen = glm::clamp(sheen, .0f, 1.f);
		sheen_power = glm::clamp(sheen_power, .0f, 1.f);
		sheen_pack = glm::packUnorm2x16({ sheen, sheen_power });
	}
	
	void set_anisotropy_and_metallicity(float ansio_ratio, float metallic) {
		ansio_ratio = (ansio_ratio - material_layer_min_ansio_ratio) / (material_layer_max_ansio_ratio - material_layer_min_ansio_ratio);
		ansio_ratio = glm::clamp(ansio_ratio, .0f, 1.f);
		metallic = glm::clamp(metallic, .0f, 1.f);
		ansi_metal_pack = glm::packUnorm2x16({ ansio_ratio, metallic });
	}
	
	void set_roughness_and_thickness(float roughness, float thickness) {
		roughness = glm::clamp(roughness, .0f, 1.f);
		thickness = glm::clamp(thickness / material_layer_max_thickness, .0f, 1.f);
		roughness_thickness_pack = glm::packUnorm2x16({ roughness, thickness });
	}

	void set_ior(float f) {
		f = glm::max(1.f, f);
		ior = f;
	}

	void set_attenuation_coefficient(float a) {
		a = glm::max(.0f, a);
		attenuation_coefficient = a;
	}

	void set_next_layer_id(std::uint32_t id) {
		next_layer_id = id;
	}
};

}
}
