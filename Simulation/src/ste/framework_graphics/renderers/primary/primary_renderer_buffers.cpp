
#include <stdafx.hpp>
#include <primary_renderer_buffers.hpp>

#include <device_pipeline_shader_stage.hpp>
#include <external_binding_set_collection_from_shader_stages.hpp>

using namespace ste;
using namespace ste::graphics;

gl::pipeline_external_binding_set primary_renderer_buffers::create_common_binding_set_collection(const ste_context &ctx,
																								 const renderer_transform_buffers &transform_buffers,
																								 const linked_light_lists &linked_light_list_storage,
																								 const deferred_gbuffer &gbuffer,
																								 const shadowmap_storage &shadows,
																								 const atmospherics_buffer &atmospheric_buffer,
																								 const atmospherics_lut_storage &atmospherics_luts,
																								 const scene *s) {
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
		std::move(v)).generate()
	};

	// Transforms buffer bindings
	set["view_transform_buffer_binding"] = gl::bind(transform_buffers.get_view_buffer());
	set["proj_transform_buffer_binding"] = gl::bind(transform_buffers.get_proj_buffer());

	// Mesh and material bindings
	set["mesh_descriptors_binding"] = gl::bind(s->get_object_group().get_draw_buffers().get_mesh_data_buffer());
	set["mesh_draw_params_binding"] = gl::bind(s->get_object_group().get_draw_buffers().get_mesh_draw_params_buffer());
	set["material_descriptors_binding"] = gl::bind(s->properties().materials_storage().buffer());
	set["material_layer_descriptors_binding"] = gl::bind(s->properties().material_layers_storage().buffer());
	set["material_sampler"] = gl::bind(ctx.device().common_samplers_collection().linear_mipmap_anisotropic16_sampler());

	// Light bindings
	set["light_binding"] = gl::bind(s->properties().lights_storage().buffer());
	set["light_list_counter_binding"] = gl::bind(s->properties().lights_storage().get_active_ll_counter());
	set["light_list_binding"] = gl::bind(s->properties().lights_storage().get_active_ll(),
										 0, light_storage::max_ll_buffer_size);
	set["shaped_lights_points_binding"] = gl::bind(s->properties().lights_storage().get_shaped_lights_points_buffer());

	set["linked_light_list_size"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_size_map()));
	set["linked_light_list_low_detail_size"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_low_detail_size_map()));
	set["linked_light_list_heads"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_heads_map()));
	set["linked_light_list_low_detail_heads"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_low_detail_heads_map()));

	set["linked_light_list_counter_binding"] = gl::bind(linked_light_list_storage.linked_light_lists_counter_buffer());
	set["linked_light_list_binding"] = gl::bind(linked_light_list_storage.linked_light_lists_buffer());

	set["cascades_depths_uniform_binding"] = gl::bind(s->properties().lights_storage().get_cascade_depths_uniform_buffer());

	// G-Buffer
	set["downsampled_depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get_downsampled_depth_target(),
																				 ctx.device().common_samplers_collection().linear_sampler()));
	set["depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get_depth_target(),
																	 ctx.device().common_samplers_collection().linear_sampler()));
	set["backface_depth_map"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get_backface_depth_target(),
																			  ctx.device().common_samplers_collection().linear_sampler()));
	set["gbuffer"] = gl::bind(gl::pipeline::combined_image_sampler(gbuffer.get_gbuffer(),
																   ctx.device().common_samplers_collection().nearest_clamp_sampler()));

	// Shadows
	set["shadow_depth_maps"] = gl::bind(gl::pipeline::combined_image_sampler(shadows.get_cubemaps(),
																			 shadows.get_shadow_sampler()));
	set["shadow_maps"] = gl::bind(gl::pipeline::combined_image_sampler(shadows.get_cubemaps(),
																	   ctx.device().common_samplers_collection().linear_clamp_sampler()));
	set["directional_shadow_depth_maps"] = gl::bind(gl::pipeline::combined_image_sampler(shadows.get_directional_maps(),
																						 shadows.get_shadow_sampler()));
	set["directional_shadow_maps"] = gl::bind(gl::pipeline::combined_image_sampler(shadows.get_directional_maps(),
																				   ctx.device().common_samplers_collection().linear_clamp_sampler()));

	// Atmospherics
	set["atmospheric_optical_length_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts.get_atmospherics_optical_length_lut(),
																						  ctx.device().common_samplers_collection().linear_clamp_sampler()));
	set["atmospheric_mie0_scattering_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts.get_atmospherics_mie0_scatter_lut(),
																						   ctx.device().common_samplers_collection().linear_clamp_sampler()));
	set["atmospheric_ambient_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts.get_atmospherics_ambient_lut(),
																				   ctx.device().common_samplers_collection().linear_clamp_sampler()));
	set["atmospheric_scattering_lut"] = gl::bind(gl::pipeline::combined_image_sampler(atmospherics_luts.get_atmospherics_scatter_lut(),
																					  ctx.device().common_samplers_collection().linear_clamp_sampler()));
	set["atmospherics_descriptor_binding"] = gl::bind(atmospheric_buffer.get());

	return set;
}

void primary_renderer_buffers::update_common_binding_set() {
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
									  scene *s,
									  const camera_t *cam) {
	// Update material bindings, if materials were mutated
	common_binding_set_collection["material_textures_count"] = s->properties().material_textures_storage().size();
	common_binding_set_collection["material_textures"] = s->properties().material_textures_storage().binder();

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
