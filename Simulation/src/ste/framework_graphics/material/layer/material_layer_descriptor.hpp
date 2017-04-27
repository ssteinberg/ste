// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>
#include <texture_handle.hpp>

namespace ste {
namespace Graphics {

static constexpr std::uint32_t material_layer_none = 0xFFFFFFFF;
static constexpr float material_layer_max_thickness = .1f;
static constexpr float material_layer_min_ior = 1.f;
static constexpr float material_layer_max_ior = 5.f;
static constexpr float material_layer_ansio_ratio_scale = .9f;
extern const float material_layer_max_ansio_ratio;
extern const float material_layer_min_ansio_ratio;

class material_layer_descriptor {
private:
	Core::texture_handle roughness_map;
	Core::texture_handle metallicity_map;
	Core::texture_handle thickness_map;

	std::uint32_t packed_albedo{ 0xFFFFFFFF };
	std::uint32_t next_layer_id{ material_layer_none };

	glm::vec3 attenuation_coefficient{ .0f };
	std::uint32_t ior_phase_pack{ 0 };

public:
	material_layer_descriptor() = default;
	
	material_layer_descriptor(material_layer_descriptor &&) = default;
	material_layer_descriptor(const material_layer_descriptor &) = default;
	material_layer_descriptor &operator=(material_layer_descriptor &&) = default;
	material_layer_descriptor &operator=(const material_layer_descriptor &) = default;

	void set_albedo(const glm::vec4 &c) {
		packed_albedo = glm::packUnorm4x8(c);
	}

	void set_roughness_map_handle(const Core::texture_handle &handle) {
		roughness_map = handle;
	}

	void set_metallicity_map_handle(const Core::texture_handle &handle) {
		metallicity_map = handle;
	}

	void set_thickness_map_handle(const Core::texture_handle &handle) {
		thickness_map = handle;
	}

	void set_ior_phase(float ior, float phase) {
		ior = (glm::clamp(ior, material_layer_min_ior, material_layer_max_ior) - material_layer_min_ior) / (material_layer_max_ior - material_layer_min_ior);
		phase = glm::clamp(phase, -1.f, +1.f) * .5f + .5f;
		ior_phase_pack = glm::packUnorm2x16(glm::vec2{ ior, phase });
	}

	void set_attenuation_coefficient(const glm::vec3 &a) {
		auto clamped_a = glm::max(glm::vec3{ .0f }, a);
		attenuation_coefficient = clamped_a;
	}

	void set_next_layer_id(std::uint32_t id) {
		next_layer_id = id;
	}
};

}
}
