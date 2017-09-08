// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <rendering_presentation_system.hpp>
#include <presentation_engine.hpp>
#include <pipeline_external_binding_set.hpp>

#include <scene.hpp>

#include <primary_renderer_camera.hpp>
#include <primary_renderer_buffers.hpp>
#include <primary_renderer_framebuffers.hpp>

#include <deferred_composer.hpp>
#include <fxaa_postprocess.hpp>
#include <hdr_dof_postprocess.hpp>
#include <scene_prepopulate_depth_fragment.hpp>
#include <scene_geo_cull_fragment.hpp>
#include <linked_light_lists_gen_fragment.hpp>
#include <light_preprocessor_fragment.hpp>
#include <shadowmap_projector.hpp>
#include <directional_shadowmap_projector.hpp>
#include <volumetric_scattering_scatter_fragment.hpp>
#include <gbuffer_downsample_depth_fragment.hpp>

#include <signal.hpp>
#include <optional.hpp>

namespace ste {
namespace graphics {

class primary_renderer : public gl::rendering_presentation_system {
	using Base = gl::rendering_presentation_system;

	using camera_t = primary_renderer_camera;

private:
	std::reference_wrapper<gl::presentation_engine> presentation;
	const camera_t *cam;
	scene *s;

	gl::ste_device::queues_and_surface_recreate_signal_type::connection_type resize_signal_connection;
	camera_t::projection_change_signal::connection_type camera_projection_change_signal;

	primary_renderer_buffers buffers;
	primary_renderer_framebuffers framebuffers;

private:
	ste_resource<deferred_composer> composer;
	ste_resource<hdr_dof_postprocess> hdr;
	ste_resource<fxaa_postprocess> fxaa;

	ste_resource<gbuffer_downsample_depth_fragment> downsample_depth;
	ste_resource<scene_prepopulate_depth_front_face_fragment> prepopulate_depth;
	ste_resource<scene_prepopulate_depth_back_face_fragment> prepopulate_backface_depth;
	ste_resource<scene_geo_cull_fragment> scene_geo_cull;

	ste_resource<linked_light_lists_gen_fragment> linked_light_list_generator;
	ste_resource<light_preprocessor_fragment> light_preprocess;

	ste_resource<shadowmap_projector> shadows_projector;
	ste_resource<directional_shadowmap_projector> directional_shadows_projector;

	ste_resource<volumetric_scattering_scatter_fragment> volumetric_scatterer;

private:
	optional<atmospherics_properties<double>> atmospherics_properties_update;

private:
	static gl::framebuffer_layout create_fb_layout(const ste_context &ctx);

	/**
	 *	@brief		Update buffers
	 */
	void update(gl::command_recorder &recorder);

	/*
	 *	Fragments' recording methods
	 */

	void record_light_preprocess_fragment(gl::command_recorder &recorder);
	void record_scene_geometry_cull_fragment(gl::command_recorder &recorder);
	void record_shadow_projector_fragment(gl::command_recorder &recorder);
	void record_directional_shadow_projector_fragment(gl::command_recorder &recorder);
	void record_prepopulate_depth_fragment(gl::command_recorder &recorder);
	void record_downsample_depth_fragment(gl::command_recorder &recorder);
	void record_linked_light_list_generator_fragment(gl::command_recorder &recorder);
	void record_scene_fragment(gl::command_recorder &recorder);
	void record_prepopulate_depth_backface_fragment(gl::command_recorder &recorder);
	void record_volumetric_scattering_fragment(gl::command_recorder &recorder);
	void record_deferred_composer_fragment(gl::command_recorder &recorder);
	void record_hdr_fragment(gl::command_recorder &recorder);
	void record_fxaa_fragment(gl::command_recorder &recorder);
	
public:
	primary_renderer(const ste_context &ctx,
					 gl::presentation_engine &presentation,
					 const camera_t *cam,
					 scene *s,
					 const atmospherics_properties<double> &atmospherics_prop);
	~primary_renderer() noexcept {}

	const gl::pipeline_external_binding_set* external_binding_set() const override final {
		return &buffers.common_binding_set_collection;
	}

	/**
	 *	@brief		Updates atmospheric properties.
	 */
	void update_atmospherics_properties(const atmospherics_properties<double> &atmospherics_prop) { atmospherics_properties_update = atmospherics_prop; }

	/**
	 *	@brief		Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	 *
	 * 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	 *	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	 */
	void set_aperture_parameters(float diameter, float focal_length) { hdr->set_aperture_parameters(diameter, focal_length); }

protected:
	/**
	 *	@brief		Performs rendering and presentation.
	 */
	void present() override final;
	void render() override final {}
};

}
}
