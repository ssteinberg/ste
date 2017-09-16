﻿
#include <stdafx.hpp>
#include <primary_renderer.hpp>

using namespace ste;
using namespace ste::graphics;

primary_renderer::primary_renderer(const ste_context &ctx,
								   gl::framebuffer_layout &&fb_layout,
								   const camera_t *cam,
								   scene *s,
								   const atmospherics_properties<double> &atmospherics_prop)
	: Base(ctx),

	cam(cam),
	s(s),

	buffers(ctx,
			ctx.device().get_surface().extent(),
			s,
			this->acquire_storage<atmospherics_lut_storage>(),
			atmospherics_prop),
	framebuffers(ctx,
				 ctx.device().get_surface().extent()),

	composer(ctx,
			 *this,
			 &buffers.vol_scat_storage.get()),
	hdr(ctx,
		*this,
		ctx.device().get_surface().extent(),
		gl::framebuffer_layout(framebuffers.fxaa_input_fb.get_layout())),
	fxaa(ctx,
		 *this,
		 std::move(fb_layout)),

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
					 *this->cam),

	shadows_projector(ctx,
					  *this,
					  this->s),
	directional_shadows_projector(ctx,
								  *this,
								  this->s),

	volumetric_scatterer(ctx,
						 *this,
						 &buffers.vol_scat_storage.get(),
						 &this->s->properties().lights_storage())
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
	camera_projection_change_signal = make_connection(cam->get_projection_change_signal(), [this](auto, auto) {
		// Update buffers and fragments that rely on projection data
		buffers.invalidate_projection_buffer();

		light_preprocess->update_projection_planes(*this->cam);
	});

	// Attach framebuffers
	reattach_framebuffers();
}

void primary_renderer::reattach_framebuffers() {
	// Attach shadow projectors' framebuffers
	shadows_projector->attach_framebuffer(buffers.shadows_storage->get_cube_fbo());
	directional_shadows_projector->attach_framebuffer(buffers.shadows_storage->get_directional_maps_fbo());

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
															  gl::buffer_memory_barrier(s->properties().lights_storage().get_directional_lights_cascades_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write),
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

	// Update directional lights' cascades based on projection 
	s->properties().lights_storage().update_directional_lights_cascades_buffer(recorder, this->cam->view_transform_dquat(),
																			   this->cam->get_projection_model().get_fovy(),
																			   this->cam->get_projection_model().get_aspect(),
																			   this->cam->get_projection_model().get_near_clip_plane());

	// Update scene, light storage and objects
	s->update_scene(recorder);

	// Update buffers
	buffers.update(recorder, cam);

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->properties().lights_storage().get_directional_lights_cascades_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read),
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
	if (atmospherics_properties_update) {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		buffers.update_atmospheric_properties(recorder,
											  atmospherics_properties_update.get());
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(buffers.atmospheric_buffer.get(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read)));

		atmospherics_properties_update = none;
	}

	// Update common binding set
	buffers.update_common_binding_set(s);
}

void primary_renderer::render(gl::command_recorder &recorder) {
	// Update data
	update(recorder);

	// Render

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
															  gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->properties().lights_storage().get_active_ll(),
																						gl::access_flags::shader_read,
																						gl::access_flags::shader_read | gl::access_flags::shader_write)));

	// Light preprocess
	record_light_preprocess_fragment(recorder);

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
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::geometry_shader,
															  gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->get_shadow_projection_buffers().proj_id_to_light_id_translation_table,
																						gl::access_flags::shader_read,
																						gl::access_flags::shader_write),
															  gl::buffer_memory_barrier(s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table,
																						gl::access_flags::shader_read,
																						gl::access_flags::shader_write)));
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::draw_indirect,
															  gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(s->get_idb().get(),
																						gl::access_flags::indirect_command_read,
																						gl::access_flags::shader_write),
															  gl::buffer_memory_barrier(s->get_shadow_projection_buffers().idb.get(),
																						gl::access_flags::indirect_command_read,
																						gl::access_flags::shader_write),
															  gl::buffer_memory_barrier(s->get_directional_shadow_projection_buffers().idb.get(),
																						gl::access_flags::indirect_command_read,
																						gl::access_flags::shader_write)));

	// Scene geometry cull
	record_scene_geometry_cull_fragment(recorder);

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::draw_indirect,
															  gl::buffer_memory_barrier(s->get_idb().get(),
																						gl::access_flags::shader_write,
																						gl::access_flags::indirect_command_read),
															  gl::buffer_memory_barrier(s->get_shadow_projection_buffers().idb.get(),
																						gl::access_flags::shader_write,
																						gl::access_flags::indirect_command_read),
															  gl::buffer_memory_barrier(s->get_directional_shadow_projection_buffers().idb.get(),
																						gl::access_flags::shader_write,
																						gl::access_flags::indirect_command_read)));
//	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
//															  gl::pipeline_stage::geometry_shader,
//															  gl::buffer_memory_barrier(s->get_shadow_projection_buffers().proj_id_to_light_id_translation_table,
//																						gl::access_flags::shader_write,
//																						gl::access_flags::shader_read)));

	// Shadow cubemaps project
//	record_shadow_projector_fragment(recorder);
//
//	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
//															  gl::pipeline_stage::geometry_shader,
//															  gl::buffer_memory_barrier(s->get_directional_shadow_projection_buffers().proj_id_to_light_id_translation_table,
//																						gl::access_flags::shader_write,
//																						gl::access_flags::shader_read)));
//
//	// Directional shadow maps project
//	record_directional_shadow_projector_fragment(recorder);

	// Draw scene to gbuffer
	record_scene_fragment(recorder);

	// TODO: Event
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::compute_shader,
															  gl::image_memory_barrier(buffers.gbuffer.get().get_downsampled_depth_target().get_image(),
																					   gl::image_layout::shader_read_only_optimal,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read,
																					   gl::access_flags::shader_write)));

	// Downsample depth
	record_downsample_depth_fragment(recorder);

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
																					   gl::access_flags::shader_read | gl::access_flags::shader_write),
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_low_detail_heads_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write),
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_low_detail_size_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write)));

	// Linked-light-list generator
	record_linked_light_list_generator_fragment(recorder);

	// Prepopulate back-face depth buffer
	record_prepopulate_depth_backface_fragment(recorder);

	// TODO: Event
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
															  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::shader_write)));
	// TODO: Event
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::compute_shader,
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_low_detail_heads_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write,
																					   gl::access_flags::shader_read),
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_low_detail_size_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write,
																					   gl::access_flags::shader_read)));

	// Volumetric scattering
	record_volumetric_scattering_fragment(recorder);

	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::fragment_shader,
															  gl::image_memory_barrier(buffers.vol_scat_storage.get().get_volume_texture().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::shader_read_only_optimal,
																					   gl::access_flags::shader_write,
																					   gl::access_flags::shader_read)));
	// TODO: Event
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader,
															  gl::pipeline_stage::fragment_shader,
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_heads_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write,
																					   gl::access_flags::shader_read),
															  gl::image_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_size_map().get_image(),
																					   gl::image_layout::general,
																					   gl::image_layout::general,
																					   gl::access_flags::shader_read | gl::access_flags::shader_write,
																					   gl::access_flags::shader_read)));

	// Deferred compose
	record_deferred_composer_fragment(recorder);

	// Post-process, HDR tonemapping and FXAA
	recorder
		<< hdr.get()
		<< fxaa.get();
}

void primary_renderer::record_light_preprocess_fragment(gl::command_recorder &recorder) {
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

	// Preprocess lights
	recorder << light_preprocess.get();
}

void primary_renderer::record_scene_geometry_cull_fragment(gl::command_recorder &recorder) {
	recorder << scene_geo_cull.get();
}

void primary_renderer::record_shadow_projector_fragment(gl::command_recorder &recorder) {
	recorder << shadows_projector.get();
}

void primary_renderer::record_directional_shadow_projector_fragment(gl::command_recorder &recorder) {
	recorder << directional_shadows_projector.get();
}

void primary_renderer::record_downsample_depth_fragment(gl::command_recorder &recorder) {
	recorder << downsample_depth.get();
}

void primary_renderer::record_linked_light_list_generator_fragment(gl::command_recorder &recorder) {
	// Clear ll counter
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::compute_shader | gl::pipeline_stage::fragment_shader,
															  gl::pipeline_stage::transfer,
															  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																						gl::access_flags::shader_read,
																						gl::access_flags::transfer_write)));
	buffers.linked_light_list_storage.get().clear(recorder);
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::compute_shader,
															  gl::buffer_memory_barrier(buffers.linked_light_list_storage.get().linked_light_lists_counter_buffer(),
																						gl::access_flags::transfer_write,
																						gl::access_flags::shader_read | gl::access_flags::shader_write)));

	// Preprocess lights
	recorder << linked_light_list_generator.get();
}

void primary_renderer::record_scene_fragment(gl::command_recorder &recorder) {
	recorder << scene_write_gbuffer.get();
}

void primary_renderer::record_prepopulate_depth_backface_fragment(gl::command_recorder &recorder) {
	recorder << prepopulate_backface_depth.get();
}

void primary_renderer::record_volumetric_scattering_fragment(gl::command_recorder &recorder) {
	// Clear scatter volume
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::fragment_shader,
															  gl::pipeline_stage::transfer,
															  gl::image_memory_barrier(buffers.vol_scat_storage.get().get_volume_texture().get_image(),
																					   gl::image_layout::shader_read_only_optimal,
																					   gl::image_layout::transfer_dst_optimal,
																					   gl::access_flags::shader_read,
																					   gl::access_flags::transfer_write)));
	volumetric_scatterer.get().clear(recorder);
	recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															  gl::pipeline_stage::compute_shader,
															  gl::image_memory_barrier(buffers.vol_scat_storage.get().get_volume_texture().get_image(),
																					   gl::image_layout::transfer_dst_optimal,
																					   gl::image_layout::general,
																					   gl::access_flags::transfer_write,
																					   gl::access_flags::shader_write)));

	// Scatter
	recorder << volumetric_scatterer.get();
}

void primary_renderer::record_deferred_composer_fragment(gl::command_recorder &recorder) {
	recorder << composer.get();
}
