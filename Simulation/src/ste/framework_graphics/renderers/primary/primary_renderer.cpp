
#include <stdafx.hpp>
#include <primary_renderer.hpp>

using namespace ste;
using namespace ste::graphics;

gl::framebuffer_layout primary_renderer::create_fb_layout(const ste_context &ctx) {
	gl::framebuffer_layout fb_layout;
	fb_layout[0] = gl::ignore_store(ctx.device().get_surface().surface_format(),
									gl::image_layout::color_attachment_optimal);
	return fb_layout;
}

primary_renderer::primary_renderer(const ste_context &ctx,
								   gl::presentation_engine &presentation,
								   const camera_t *cam,
								   scene *s,
								   const atmospherics_properties<double> &atmospherics_prop)
	: Base(ctx,
		   create_fb_layout(ctx)),

	presentation(presentation),
	cam(cam),
	s(s),

	buffers(ctx,
			ctx.device().get_surface().extent(),
			s,
			this->acquire_storage<atmospherics_lut_storage>(),
			atmospherics_prop),
	framebuffers(ctx,
				 ctx.device().get_surface().extent()),

	composer(*this),
	hdr(*this,
		ctx.device().get_surface().extent(),
		gl::framebuffer_layout(framebuffers.fxaa_input_fb.get_layout())),
	fxaa(*this,
		 gl::framebuffer_layout(framebuffers.hdr_input_fb.get_layout())),

	downsample_depth(*this,
					 &buffers.gbuffer.get()),
	prepopulate_depth(*this,
					  s),
	prepopulate_backface_depth(*this,
							   s),
	scene_geo_cull(*this,
				   s, &s->properties().lights_storage()),

	linked_light_list_generator(*this,
								&buffers.linked_light_list_storage.get()),
	light_preprocess(*this,
					 &s->properties().lights_storage(),
					 *cam),

	shadows_projector(*this,
					  s),
	directional_shadows_projector(*this,
								  s),

	volumetric_scatterer(*this,
						 &buffers.vol_scat_storage.get(),
						 &s->properties().lights_storage())
{
	// Attach a connection to swapchain's surface resize signal
	resize_signal_connection = make_connection(ctx.device().get_queues_and_surface_recreate_signal(), [this, &ctx](auto) {
		// Resize buffers and framebuffers
		buffers.resize(ctx.device().get_surface().extent());
		framebuffers.resize(ctx.device().get_surface().extent());

		// Send resize signal to interested fragments
		hdr.resize(device().get_surface().extent());

		// Reattach resized framebuffers and input images
		hdr.attach_framebuffer(framebuffers.fxaa_input_fb);
		hdr.set_input_image(&framebuffers.hdr_input_image.get());
		fxaa.set_input_image(&framebuffers.fxaa_input_image.get());
	});

	// Attach a connection to camera's projection change signal
	camera_projection_change_signal = make_connection(cam->get_projection_change_signal(), [this](auto, auto) {
		// Update buffers and fragments that rely on projection data
		buffers.invalidate_projection_buffer();

		light_preprocess.update_projection_planes(*this->cam);
	});
}

void primary_renderer::update(gl::command_recorder &recorder) {
	// Update directional lights' cascades based on projection 
	s->properties().lights_storage().update_directional_lights_cascades_buffer(recorder, this->cam->view_transform_dquat(),
																			   this->cam->get_projection_model().get_fovy(),
																			   this->cam->get_projection_model().get_aspect(),
																			   this->cam->get_projection_model().get_near_clip_plane());

	// Update buffers
	buffers.update(recorder,
				   s, cam);

	// Update atmospheric properties (if needed)
	if (atmospherics_properties_update) {
		buffers.update_atmospheric_properties(recorder,
											  atmospherics_properties_update.get());
		atmospherics_properties_update = none;
	}

	// Update common binding set
	buffers.update_common_binding_set();
}

void primary_renderer::present() {
	auto selector = gl::make_queue_selector(gl::ste_queue_type::primary_queue);

	// Acquire presentation comand batch
	auto batch = presentation.get().allocate_presentation_command_batch(selector);

	// Record and submit a batch
	device().enqueue(selector, [this, batch = std::move(batch)]() mutable {
		auto& command_buffer = batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();

			// Attach swap chain framebuffer to last stage, fxaa
			auto &fb = swap_chain_framebuffer(batch->presentation_image_index());
			fxaa.attach_framebuffer(fb);

			// Update data
			update(recorder);

			// Render
			// light_preprocess → scene_geo_cull → shadow_projector → directional_shadow_projector → prepopulate_depth → downsample_depth → lll_gen → scene → prepopulate_depth_backface → volumetric_scattering → composer → hdr → fxaa
			recorder
				<< hdr
				<< fxaa

				<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::color_attachment_output,
																 gl::pipeline_stage::bottom_of_pipe,
																 gl::image_layout_transform_barrier(swap_chain_image(batch->presentation_image_index()).image,
																									gl::image_layout::color_attachment_optimal,
																									gl::image_layout::present_src_khr)));
		}

		// Submit command buffer and present
		presentation.get().submit_and_present(std::move(batch));
	});
}
