
#include <stdafx.hpp>
#include <primary_renderer.hpp>

#include <external_binding_set_collection_from_shader_stages.hpp>

using namespace ste;
using namespace ste::graphics;

gl::framebuffer_layout primary_renderer::create_fb_layout(const ste_context &ctx) {
	gl::framebuffer_layout fb_layout;
	fb_layout[0] = gl::ignore_store(ctx.device().get_surface().surface_format(),
									gl::image_layout::color_attachment_optimal);
	return fb_layout;
}

gl::pipeline_external_binding_set_collection primary_renderer::create_common_binding_set_collection(const ste_context &ctx,
																									const scene *s) {
	gl::device_pipeline_shader_stage common_bindings_spirv(ctx, "primary_renderer_common_descriptor_sets.comp");
	gl::external_binding_set_collection_from_shader_stages external_binding_set_collection_generator(ctx.device(), {});
	auto set = gl::pipeline_external_binding_set_collection(std::move(external_binding_set_collection_generator).generate());

	// Mesh and material bindings
	set["mesh_descriptors_binding"] = gl::bind(s->get_object_group().get_draw_buffers().get_mesh_data_buffer());
	set["material_descriptors_binding"] = gl::bind(s->properties().materials_storage().buffer());
	set["material_layer_descriptors_binding"] = gl::bind(s->properties().material_layers_storage().buffer());

	// Light bindings

	return set;
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

	transform_buffers(ctx),
	atmospheric_buffer(ctx, atmospherics_prop),

	lll_storage(ctx, ctx.device().get_surface().extent()),
	shadows_storage(ctx),
	vol_scat_storage(ctx),

	common_binding_set_collection(create_common_binding_set_collection(ctx,
																	   s)),

	composer(ctx),
	fxaa(ctx),
	hdr(ctx),

	downsample_depth(ctx),
	prepopulate_depth_dispatch(ctx),
	prepopulate_backface_depth_dispatch(ctx),
	scene_geo_cull(ctx),

	lll_gen_dispatch(ctx),
	light_preprocess(ctx),

	shadows_projector(ctx),
	directional_shadows_projector(ctx),

	vol_scat_scatter(ctx) 
{
}

void primary_renderer::update(gl::command_recorder &recorder) {
	// Update material bindings, in materials were mutated
	common_binding_set_collection["material_samplers"] = s->properties().materials_storage().get_material_texture_storage().binder();

	// Upload new camera and projection transform data
	transform_buffers.update_view_data(recorder, *this->cam);
	transform_buffers.update_proj_data(recorder, *this->cam, device().get_surface().extent());

	// Update directional lights' cascades based on projection 
	s->properties().lights_storage().update_directional_lights_cascades_buffer(recorder,
																			   *this->cam,
																			   this->cam->get_projection_model().get_fov(), 
																			   this->cam->get_projection_model().get_projection_aspect(), 
																			   this->cam->get_projection_model().get_near_clip_plane());
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

			// Update
			update(recorder);

			// Attach swap chain framebuffer to last stage, fxaa
			auto &fb = swap_chain_framebuffer(batch->presentation_image_index());
			fxaa.attach_framebuffer(fb);

			// Render
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

