
#include "stdafx.hpp"
#include "material_layer.hpp"

using namespace StE::Graphics;

material_layer::material_layer() {
	material_sampler.set_wrap_s(Core::texture_wrap_mode::Wrap);
	material_sampler.set_wrap_t(Core::texture_wrap_mode::Wrap);
	material_sampler.set_wrap_r(Core::texture_wrap_mode::Wrap);
	material_sampler.set_min_filter(Core::texture_filtering::Linear);
	material_sampler.set_mag_filter(Core::texture_filtering::Linear);
	material_sampler.set_mipmap_filter(Core::texture_filtering::Linear);
	material_sampler.set_anisotropic_filter(16);
	
	// Write default values
	descriptor.set_ior_phase(index_of_refraction, phase_g);
	descriptor.set_attenuation_coefficient(attenuation_coefficient);

	write_scalar_map<&material_layer::roughness_map>(.5f);
	descriptor.set_roughness_map_handle(handle_for_texture(roughness_map.get()));

//	write_scalar_map<&material_layer::anisotropy_map>(.0f);
//	descriptor.set_anisotropy_map_handle(handle_for_texture(anisotropy_map.get()));

	write_scalar_map<&material_layer::metallicity_map>(.0f);
	descriptor.set_metallicity_map_handle(handle_for_texture(metallicity_map.get()));

	write_scalar_map<&material_layer::thickness_map>(.0f);
	descriptor.set_thickness_map_handle(handle_for_texture(thickness_map.get()));
}

StE::Core::texture_handle material_layer::handle_for_texture(const Core::texture_2d *t) const {
	Core::texture_handle h;
	if (t != nullptr) {
		h = t->get_texture_handle(material_sampler);
		h.make_resident();
	}
	return h;
}
