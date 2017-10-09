
#include <stdafx.hpp>
#include <primary_renderer.hpp>
#include "host_read_buffer.hpp"

using namespace ste;
using namespace ste::graphics;

namespace ste::graphics::_detail {

template <typename F>
void primary_renderer_atom(gl::profiler::profiler *profiler,
						   gl::command_recorder &recorder,
						   lib::string name,
						   gl::pipeline_stage stages,
						   F &&f) {
	if (profiler) {
		auto atom = profiler->start_atom(recorder,
										 name,
										 stages);
		f();
	}
	else {
		f();
	}
}
template <typename F>
void primary_renderer_atom(gl::profiler::profiler *profiler,
						   gl::command_recorder &recorder,
						   lib::string name,
						   F &&f) {
	primary_renderer_atom(profiler, recorder, name, gl::pipeline_stage::bottom_of_pipe, std::forward<F>(f));
}

}

primary_renderer::primary_renderer(const ste_context &ctx,
								   gl::framebuffer_layout &&fb_layout,
								   const camera_t *cam,
								   scene *s,
								   const atmospherics_properties<double> &atmospherics_prop,
								   voxels_configuration voxel_config,
								   gl::profiler::profiler *profiler)
	: Base(ctx),
	profiler(profiler),

	cam(cam),
	s(s),

	buffers(ctx,
			ctx.device().get_surface().extent(),
			s,
			this->acquire_storage<atmospherics_lut_storage>(),
			atmospherics_prop,
			voxel_config),
	framebuffers(ctx,
				 ctx.device().get_surface().extent()),

	composer(ctx,
			 *this),
	hdr(ctx,
		*this,
		ctx.device().get_surface().extent(),
		gl::framebuffer_layout(framebuffers.fxaa_input_fb.get_layout())),
	fxaa(ctx,
		 *this,
		 std::move(fb_layout)),

	voxelizer(ctx,
			  *this,
			  &buffers.voxels.get(),
			  this->s),

	downsample_depth(ctx,
					 *this,
					 &buffers.gbuffer.get()),
	prepopulate_backface_depth(ctx,
							   *this,
							   this->s),
	scene_write_gbuffer(ctx,
						*this,
						this->s, &buffers.gbuffer.get()),
	scene_geo_cull(ctx,
				   *this,
				   this->s, &this->s->properties().lights_storage()),

	linked_light_list_generator(ctx,
								*this,
								&buffers.linked_light_list_storage.get()),
	light_preprocess(ctx,
					 *this,
					 &this->s->properties().lights_storage(),
					 this->cam->get_projection_model())
{
	// Attach a connection to swapchain's surface resize signal
	resize_signal_connection = make_connection(ctx.device().get_queues_and_surface_recreate_signal(), [this, &ctx](auto) {
		// Resize buffers and framebuffers
		buffers.resize(ctx.device().get_surface().extent());
		framebuffers.resize(ctx.device().get_surface().extent());

		// Send resize signal to interested fragments
		hdr->resize(device().get_surface().extent());

		// Reattach resized framebuffers and input images
		reattach_framebuffers();
	});

	// Attach a connection to camera's projection change signal
	camera_projection_change_signal = make_connection(cam->get_projection_change_signal(), [this](auto, auto projection) {
		// Update buffers and fragments that rely on projection data
		buffers.invalidate_projection_buffer();

		light_preprocess->update_projection_planes(projection);
	});

	// Attach framebuffers
	reattach_framebuffers();
}

void primary_renderer::reattach_framebuffers() {
	// Attach gbuffer framebuffers
	prepopulate_backface_depth->attach_framebuffer(buffers.gbuffer->get_depth_backface_fbo());
	scene_write_gbuffer->attach_framebuffer(buffers.gbuffer->get_fbo());

	// Attach composer and hdr outputs
	composer->attach_framebuffer(framebuffers.hdr_input_fb);
	hdr->attach_framebuffer(framebuffers.fxaa_input_fb);
	hdr->set_input_image(&framebuffers.hdr_input_image.get());
	fxaa->set_input_image(&framebuffers.fxaa_input_image.get());
}

void primary_renderer::update(gl::command_recorder &recorder) {
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(s->properties().materials_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(s->properties().material_layers_storage().buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
															  gl::buffer_memory_barrier(buffers.transform_buffers.get_view_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write)));

	// Update scene, light storage and objects
	s->update_scene(recorder);

	// Update buffers
	buffers.update(recorder, cam);

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read | gl::access_flags::shader_write),
															  gl::buffer_memory_barrier(s->properties().materials_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(s->properties().material_layers_storage().buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
															  gl::buffer_memory_barrier(buffers.transform_buffers.get_view_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read)));

	// Update atmospheric properties (if needed)
	auto atmoshperics_update = atmospherics_properties_update.get();
	if (atmoshperics_update) {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.update_atmospheric_properties(recorder,
											  atmoshperics_update.get());
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read)));
	}

	// Update common binding set
	buffers.update_common_binding_set(s);
}

void primary_renderer::render(gl::command_recorder &recorder) {
	// Update data
	_detail::primary_renderer_atom(profiler, recorder, "update", 
								   [this, &recorder]() {
		update(recorder);
	});

	// Render

	_detail::primary_renderer_atom(profiler, recorder, "-> preprocess_light",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Light preprocess
	record_light_preprocess_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> geo_cull",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(s->properties().lights_storage().buffer(),
																							gl::access_flags::shader_read | gl::access_flags::shader_write,
																							gl::access_flags::shader_read)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::draw_indirect,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->get_idb().get(),
																							gl::access_flags::indirect_command_read,
																							gl::access_flags::shader_write)));
	});

	// Scene geometry cull
	record_scene_geometry_cull_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> scene",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::draw_indirect,
																  gl::buffer_memory_barrier(s->get_idb().get(),
																							gl::access_flags::shader_write,
																							gl::access_flags::indirect_command_read)));
	});

	// Draw scene to gbuffer
	record_scene_fragment(recorder);

	// Voxelize scene
	record_voxelizer_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> downsample_depth",
								   [this, &recorder]() {
		// TODO: Event
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::image_memory_barrier(buffers.gbuffer.get().get_downsampled_depth_target().get_image(),
																						   gl::image_layout::shader_read_only_optimal,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_write)));
	});

	// Downsample depth
	record_downsample_depth_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> lll",
								   [this, &recorder]() {
		// TODO: Event
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::compute_shader,
																  gl::image_memory_barrier(buffers.gbuffer.get().get_downsampled_depth_target().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::shader_read_only_optimal,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read),
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_write),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_heads_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_size_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_read,
																						   gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Linked-light-list generator
	record_linked_light_list_generator_fragment(recorder);

	// Prepopulate back-face depth buffer
	record_prepopulate_depth_backface_fragment(recorder);

	_detail::primary_renderer_atom(profiler, recorder, "-> deferred",
								   [this, &recorder]() {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::shader_write)));
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_heads_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read),
																  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_size_map().get_image(),
																						   gl::image_layout::general,
																						   gl::image_layout::general,
																						   gl::access_flags::shader_write,
																						   gl::access_flags::shader_read)));
	});

	// Deferred compose
	record_deferred_composer_fragment(recorder);

	// Post-process, HDR tonemapping and FXAA
	_detail::primary_renderer_atom(profiler, recorder, "hdr",
								   [this, &recorder]() {
		recorder << hdr.get();
	});
	_detail::primary_renderer_atom(profiler, recorder, "fxaa",
								   [this, &recorder]() {
		recorder << fxaa.get();
	});

	if (profiler)
		profiler->end_segment();
}

void primary_renderer::record_light_preprocess_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear ll",
								   [this, &recorder]() {
		// Clear ll counter
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		s->properties().lights_storage().clear_active_ll(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll_counter(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Preprocess lights
	_detail::primary_renderer_atom(profiler, recorder, "preprocess_light",
								   [this, &recorder]() {
		recorder << light_preprocess.get();
	});
}

void primary_renderer::record_scene_geometry_cull_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "geo_cull",
								   [this, &recorder]() {
		recorder << scene_geo_cull.get();
	});
}

void primary_renderer::record_voxelizer_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear voxels",
								   [this, &recorder]() {
		// Clear voxel buffer
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write),
																  gl::buffer_memory_barrier(buffers.voxels->voxels_counter_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.voxels.get().clear(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.voxels->voxels_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write),
																  gl::buffer_memory_barrier(buffers.voxels->voxels_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Voxelize
	_detail::primary_renderer_atom(profiler, recorder, "voxelizer",
								   [this, &recorder]() {
		recorder << voxelizer.get();
	});
}

void primary_renderer::record_downsample_depth_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "downsample_depth",
								   [this, &recorder]() {
		recorder << downsample_depth.get();
	});
}

void primary_renderer::record_linked_light_list_generator_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "clear lll",
								   [this, &recorder]() {
		// Clear lll counter
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.linked_light_list_storage.get().clear(recorder);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
																  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read | gl::access_flags::shader_write)));
	});

	// Preprocess lights
	_detail::primary_renderer_atom(profiler, recorder, "lll",
								   [this, &recorder]() {
		recorder << linked_light_list_generator.get();
	});
}

void primary_renderer::record_scene_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "gbuffer",
								   [this, &recorder]() {
		recorder << scene_write_gbuffer.get();
	});
}

void primary_renderer::record_prepopulate_depth_backface_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "backface_depth",
								   [this, &recorder]() {
		recorder << prepopulate_backface_depth.get();
	});
}

void primary_renderer::record_deferred_composer_fragment(gl::command_recorder &recorder) {
	_detail::primary_renderer_atom(profiler, recorder, "deferred",
								   [this, &recorder]() {
		recorder << composer.get();
	});
}
