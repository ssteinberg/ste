//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <rendering_system.hpp>
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
#include <scene_write_gbuffer_fragment.hpp>
#include <linked_light_lists_gen_fragment.hpp>
#include <light_preprocessor_fragment.hpp>
#include <gbuffer_downsample_depth_fragment.hpp>

#include <profiler.hpp>

#include <signal.hpp>
#include <optional.hpp>
#include <mutex>
#include <lib/aligned_padded_ptr.hpp>

namespace ste {
namespace graphics {

class primary_renderer : public gl::rendering_system {
	using Base = gl::rendering_system;

private:
	class atmospherics_properties_update_t {
	private:
		std::mutex m;
		optional<atmospherics_properties<double>> update;

	public:
		void operator=(const atmospherics_properties<double> &p) {
			std::unique_lock<std::mutex> l(m);
			update = p;
		}
		optional<atmospherics_properties<double>> get() {
			std::unique_lock<std::mutex> l(m);
			optional<atmospherics_properties<double>> ret = update;
			update = none;

			return ret;
		}
	};

public:
	using camera_t = primary_renderer_camera;

private:
	gl::profiler::profiler *profiler;

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
	ste_resource<scene_prepopulate_depth_back_face_fragment> prepopulate_backface_depth;
	ste_resource<scene_write_gbuffer_fragment> scene_write_gbuffer;
	ste_resource<scene_geo_cull_fragment> scene_geo_cull;

	ste_resource<linked_light_lists_gen_fragment> linked_light_list_generator;
	ste_resource<light_preprocessor_fragment> light_preprocess;

private:
	lib::aligned_padded_ptr<atmospherics_properties_update_t> atmospherics_properties_update;

private:
	void reattach_framebuffers();

	/**
	 *	@brief		Update buffers
	 */
	void update(gl::command_recorder &recorder);

	/*
	 *	Fragments' recording methods
	 */

	void record_light_preprocess_fragment(gl::command_recorder &recorder);
	void record_scene_geometry_cull_fragment(gl::command_recorder &recorder);
	void record_downsample_depth_fragment(gl::command_recorder &recorder);
	void record_linked_light_list_generator_fragment(gl::command_recorder &recorder);
	void record_scene_fragment(gl::command_recorder &recorder);
	void record_prepopulate_depth_backface_fragment(gl::command_recorder &recorder);
	void record_deferred_composer_fragment(gl::command_recorder &recorder);
	
public:
	primary_renderer(const ste_context &ctx,
					 gl::framebuffer_layout &&fb_layout,
					 const camera_t *cam,
					 scene *s,
					 const atmospherics_properties<double> &atmospherics_prop,
					 gl::profiler::profiler *profiler = nullptr);
	~primary_renderer() noexcept {}

	const gl::pipeline_external_binding_set* external_binding_set() const override final {
		return &buffers.common_binding_set_collection;
	}

	/**
	 *	@brief		Updates atmospheric properties.
	 */
	void update_atmospherics_properties(const atmospherics_properties<double> &atmospherics_prop) {
		*atmospherics_properties_update = atmospherics_prop;
	}

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) { hdr->set_aperture_parameters(diameter, focal_length); }

	/**
	*	@brief	Set the "gamma" value for the final HDR tonemapping power-law expression.
	*			>1 values compresses light regions, making the overall scene darker.
	*
	* 	@param gamma	Gamma value. Defaults to 2.2.
	*/
	void set_gamma(float gamma) { hdr->set_gamma(gamma); }

	/*
	 *	@brief		Attaches output framebuffer
	 */
	void attach_framebuffer(gl::framebuffer &fb) {
		fxaa->attach_framebuffer(fb);
	}

	/**
	 *	@brief		Performs rendering and presentation.
	 */
	void render(gl::command_recorder &) override final;
};

}
}
