
#include <stdafx.hpp>
#include <deferred_composer.hpp>

using namespace ste::graphics;

//void deferred_composer::attach_handles() const {
//	auto scattering_volume = dr->vol_scat_storage.get_volume_texture();
//	if (scattering_volume) {
//		auto scattering_volume_handle = scattering_volume->get_texture_handle(dr->vol_scat_storage.get_volume_sampler());
//		scattering_volume_handle.make_resident();
//		program.get().set_uniform("scattering_volume", scattering_volume_handle);
//	}
//
//	auto shadow_depth_maps = dr->shadows_storage.get_cubemaps();
//	if (shadow_depth_maps) {
//		auto shadow_depth_maps_handle = shadow_depth_maps->get_texture_handle(dr->shadows_storage.get_shadow_sampler());
//		shadow_depth_maps_handle.make_resident();
//		program.get().set_uniform("shadow_depth_maps", shadow_depth_maps_handle);
//
//		auto shadow_maps_handle = shadow_depth_maps->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//		shadow_maps_handle.make_resident();
//		program.get().set_uniform("shadow_maps", shadow_maps_handle);
//	}
//
//	auto directional_shadow_depth_maps = dr->shadows_storage.get_directional_maps();
//	if (directional_shadow_depth_maps) {
//		auto directional_shadow_depth_maps_handle = directional_shadow_depth_maps->get_texture_handle(dr->shadows_storage.get_shadow_sampler());
//		directional_shadow_depth_maps_handle.make_resident();
//		program.get().set_uniform("directional_shadow_depth_maps", directional_shadow_depth_maps_handle);
//
//		auto directional_shadow_maps_handle = directional_shadow_depth_maps->get_texture_handle(*Core::sampler::sampler_linear_clamp());
//		directional_shadow_maps_handle.make_resident();
//		program.get().set_uniform("directional_shadow_maps", directional_shadow_maps_handle);
//	}
//
//	program.get().set_uniform("cascades_depths", dr->s->properties().lights_storage().get_cascade_depths_array());
//}
