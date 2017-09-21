
#include <stdafx.hpp>
#include <primary_renderer_buffers.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <external_binding_set_collection_from_shader_stages.hpp>

using namespace ste;
using namespace ste::graphics;

gl::pipeline_external_binding_set primary_renderer_buffers::create_common_binding_set_collection(const ste_context &ctx) {
	// Load common descriptor sets from pre-compiled SPIR-v shader
	gl::external_binding_set_collection_from_shader_stages::shader_stages_input_vector_t v;
	{
		gl::device_pipeline_shader_stage common_bindings_spirv(ctx, "primary_renderer_common_descriptor_sets.comp");
		gl::pipeline_binding_stages_collection stages = {
			gl::ste_shader_program_stage::compute_program,
			gl::ste_shader_program_stage::vertex_program,
			gl::ste_shader_program_stage::geometry_program,
			gl::ste_shader_program_stage::fragment_program
		};

		v.emplace_back(stages, std::move(common_bindings_spirv));
	}
	gl::pipeline_external_binding_set set{
		gl::external_binding_set_collection_from_shader_stages(ctx.device(),
															   std::move(v),
															   "primary_renderer common binding set").generate()
	};

	return set;
}

void primary_renderer_buffers::common_binding_set_bind_transform_buffers() {
	// Transforms buffer bindings
	common_binding_set_collection["view_transform_buffer_binding"] = gl::bind(transform_buffers.get_view_buffer());
	common_binding_set_collection["proj_transform_buffer_binding"] = gl::bind(transform_buffers.get_proj_buffer());
}

void primary_renderer_buffers::common_binding_set_bind_mesh_and_materials() {
	// Mesh and material bindings
	common_binding_set_collection["mesh_descriptors_binding"] = gl::bind(s->get_object_group().get_draw_buffers().get_mesh_data_buffer());
	common_binding_set_collection["mesh_draw_params_binding"] = gl::bind(s->get_object_group().get_draw_buffers().get_mesh_draw_params_buffer());
	common_binding_set_collection["material_descriptors_binding"] = gl::bind(s->properties().materials_storage().buffer());
	common_binding_set_collection["material_layer_descriptors_binding"] = gl::bind(s->properties().material_layers_storage().buffer());
	common_binding_set_collection["material_sampler"] = gl::bind(ctx.get().device().common_samplers_collection().linear_mipmap_anisotropic16_sampler());
}

void primary_renderer_buffers::common_binding_set_bind_light_buffers() {
	// Light bindings
	common_binding_set_collection["light_binding"] = gl::bind(s->properties().lights_storage().buffer());
	common_binding_set_collection["light_list_counter_binding"] = gl::bind(s->properties().lights_storage().get_active_ll_counter());
	common_binding_set_collection["light_list_binding"] = gl::bind(s->properties().lights_storage().get_active_ll());
	common_binding_set_collection["shaped_lights_points_binding"] = gl::bind(s->properties().lights_storage().get_shaped_lights_points_buffer());

}

void primary_renderer_buffers::common_binding_set_bind_lll_buffers() {
	// LLL bindings
	common_binding_set_collection["linked_light_list_size"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.get().linked_light_lists_size_map()));
	common_binding_set_collection["linked_light_list_heads"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.get().linked_light_lists_heads_map()));

	common_binding_set_collection["linked_light_list_counter_binding"] = gl::bind(linked_light_list_storage.get().linked_light_lists_counter_buffer());
	common_binding_set_collection["linked_light_list_binding"] = gl::bind(linked_light_list_storage.get().linked_light_lists_buffer());
}

void primary_renderer_buffers::common_binding_set_bind_gbuffer() {
	// G-Buffer
	common_binding_set_collection["downsampled_depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get().get_downsampled_depth_target(),
																										   ctx.get().device().common_samplers_collection().linear_sampler()));
	common_binding_set_collection["depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get().get_depth_target(),
																							   ctx.get().device().common_samplers_collection().linear_sampler()));
	common_binding_set_collection["backface_depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get().get_backface_depth_target(),
																										ctx.get().device().common_samplers_collection().linear_sampler()));
	common_binding_set_collection["gbuffer"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get().get_gbuffer(),
																							 ctx.get().device().common_samplers_collection().nearest_clamp_sampler()));
}

void primary_renderer_buffers::common_binding_set_bind_atmospheric_buffers() {
	// Atmospherics
	common_binding_set_collection["atmospheric_optical_length_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts->get_atmospherics_optical_length_lut(),
																													ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
	common_binding_set_collection["atmospheric_mie0_scattering_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts->get_atmospherics_mie0_scatter_lut(),
																													 ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
	common_binding_set_collection["atmospheric_ambient_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts->get_atmospherics_ambient_lut(),
																											 ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
	common_binding_set_collection["atmospheric_scattering_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts->get_atmospherics_scatter_lut(),
																												ctx.get().device().common_samplers_collection().linear_clamp_sampler()));
	common_binding_set_collection["atmospherics_descriptor_binding"] = gl::bind(atmospheric_buffer.get());
}

void primary_renderer_buffers::update_common_binding_set(scene *s) {
	// Update material bindings, if materials were mutated
	common_binding_set_collection["material_textures_count"] = s->properties().material_textures_storage().size();
	common_binding_set_collection["material_textures"] = s->properties().material_textures_storage().binder();

	// Recreate common binding set, if invalidated.
	gl::device_pipeline_resources_marked_for_deletion old_resources;
	if (common_binding_set_collection.is_invalidated()) {
		old_resources.external_binding_set = common_binding_set_collection.recreate_set(ctx.get().device(),
																						&old_resources.binding_set_layouts);
	}
	// Update and write new resources to the common set
	common_binding_set_collection.update();

	if (old_resources) {
		// If we have any old resources, dispose of them
		ctx.get().device().pipeline_disposer().queue_deletion(std::move(old_resources));
	}
}

void primary_renderer_buffers::update(gl::command_recorder &recorder,
									  const camera_t *cam) {
	// Upload new camera transform data
	transform_buffers.update_view_data(recorder, *cam);

	// If needed, upload new camera projection data (after resize)
	if (!projection_data_up_to_date_flag.test_and_set(std::memory_order_acquire)) {
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::pipeline_stage::transfer,
																  gl::buffer_memory_barrier(transform_buffers.get_proj_buffer(),
																							gl::access_flags::shader_read,
																							gl::access_flags::transfer_write)));
		transform_buffers.update_proj_data(recorder, *cam, extent);
		recorder << gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
																  gl::pipeline_stage::vertex_shader | gl::pipeline_stage::fragment_shader | gl::pipeline_stage::compute_shader,
																  gl::buffer_memory_barrier(transform_buffers.get_proj_buffer(),
																							gl::access_flags::transfer_write,
																							gl::access_flags::shader_read)));
	}
}
