
#include <stdafx.hpp>
#include <material_layer.hpp>

using namespace ste;
using namespace ste::graphics;

material_layer::material_layer(const ste_context &ctx,
							   texture_storage &material_texture_storage)
	: ctx(ctx),
	material_texture_storage(material_texture_storage),
	material_sampler(ctx.device(),
					 gl::sampler_parameter::anisotropy(16),
					 gl::sampler_parameter::address_mode(gl::sampler_address_mode::repeat,
														 gl::sampler_address_mode::repeat,
														 gl::sampler_address_mode::repeat),
					 gl::sampler_parameter::filtering(gl::sampler_filter::linear,
													  gl::sampler_filter::linear,
													  gl::sampler_mipmap_mode::linear))
{
	// Write default values
	descriptor.set_ior_phase(index_of_refraction, phase_g);
	descriptor.set_attenuation_coefficient(attenuation_coefficient);

	roughness_map = this->material_texture_storage->allocate_slot(create_scalar_map(.5f));
	descriptor.set_roughness_map_handle(roughness_map->get_slot_idx());

//	anisotropy_map = this->material_texture_storage->allocate_slot(create_scalar_map(.0f));
//	descriptor.set_anisotropy_map_handle(anisotropy_map->get_slot_idx());

	metallicity_map = this->material_texture_storage->allocate_slot(create_scalar_map(.0f));
	descriptor.set_metallicity_map_handle(metallicity_map->get_slot_idx());

	thickness_map = this->material_texture_storage->allocate_slot(create_scalar_map(.0f));
	descriptor.set_thickness_map_handle(thickness_map->get_slot_idx());
}
