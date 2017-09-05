//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <std430.hpp>

namespace ste {
namespace graphics {

static constexpr std::uint32_t material_layer_none = 0xFFFFFFFF;
static constexpr float material_layer_max_thickness = .1f;
static constexpr float material_layer_min_ior = 1.f;
static constexpr float material_layer_max_ior = 5.f;
static constexpr float material_layer_ansio_ratio_scale = .9f;
extern const float material_layer_max_ansio_ratio;
extern const float material_layer_min_ansio_ratio;

class material_layer_descriptor : public gl::std430<std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, glm::vec4, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t> {
private:
	auto& roughness_map() { return get<0>(); }
	auto& metallicity_map() { return get<1>(); }
	auto& thickness_map() { return get<2>(); }

	auto& next_layer_id() { return get<3>(); }


	auto& attenuation_coefficient() { return get<4>(); }

	auto& packed_albedo() { return get<5>(); }
	auto& ior_phase_pack() { return get<6>(); }

public:
	material_layer_descriptor() = default;
	
	material_layer_descriptor(material_layer_descriptor &&) = default;
	material_layer_descriptor(const material_layer_descriptor &) = default;
	material_layer_descriptor &operator=(material_layer_descriptor &&) = default;
	material_layer_descriptor &operator=(const material_layer_descriptor &) = default;

	void set_albedo(const glm::vec4 &c) {
		packed_albedo() = glm::packUnorm4x8(c);
	}

	void set_roughness_map_handle(const std::uint32_t &handle) {
		roughness_map() = handle;
	}

	void set_metallicity_map_handle(const std::uint32_t &handle) {
		metallicity_map() = handle;
	}

	void set_thickness_map_handle(const std::uint32_t &handle) {
		thickness_map() = handle;
	}

	void set_ior_phase(float ior, float phase) {
		ior = (glm::clamp(ior, material_layer_min_ior, material_layer_max_ior) - material_layer_min_ior) / (material_layer_max_ior - material_layer_min_ior);
		phase = glm::clamp(phase, -1.f, +1.f) * .5f + .5f;
		ior_phase_pack() = glm::packUnorm2x16(glm::vec2{ ior, phase });
	}

	void set_attenuation_coefficient(const glm::vec3 &a) {
		const auto clamped_a = glm::max(glm::vec3{ .0f }, a);
		attenuation_coefficient() = glm::vec4{ clamped_a.x, clamped_a.y, clamped_a.z, .0f };
	}

	void set_next_layer_id(std::uint32_t id) {
		next_layer_id() = id;
	}
};

}
}
