
#include <stdafx.hpp>
#include <material_layer.hpp>

using namespace ste;
using namespace ste::graphics;

material_layer::material_layer(const ste_context &ctx,
							   material_textures_storage &textures_storage)
	: ctx(ctx),
	  textures_storage(textures_storage)
{
	// Write default values
	descriptor.set_ior_phase(index_of_refraction, phase_g);
	descriptor.set_attenuation_coefficient(attenuation_coefficient);
	descriptor.set_next_layer_id(material_layer_none);
	descriptor.set_albedo(glm::vec4{ 1.f });

	set_default_maps();
}

void material_layer::set_default_maps() {
	auto blank = this->textures_storage->blank_texture();

	roughness_map = blank;
	descriptor.set_roughness_map_handle(roughness_map.texture_index());
	roughness_map_update_data.has_mutable_map = true;

	metallicity_map = blank;
	descriptor.set_metallicity_map_handle(metallicity_map.texture_index());
	metallicity_map_update_data.has_mutable_map = true;
	
	thickness_map = blank;
	descriptor.set_thickness_map_handle(thickness_map.texture_index());
	thickness_map_update_data.has_mutable_map = true;

//	anisotropy_map = blank;
//	descriptor.set_anisotropy_map_handle(anisotropy_map.texture_index());
//	anisotropy_map_update_data.has_mutable_map = true;
}
