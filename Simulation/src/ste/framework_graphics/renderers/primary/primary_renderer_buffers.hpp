// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <pipeline_external_binding_set_collection.hpp>
#include <external_binding_set_collection_from_shader_stages.hpp>
#include <device_pipeline_shader_stage.hpp>
#include <scene.hpp>

#include <deferred_gbuffer.hpp>
#include <atmospherics_buffer.hpp>
#include <renderer_transform_buffers.hpp>
#include <linked_light_lists.hpp>
#include <shadowmap_storage.hpp>
#include <volumetric_scattering_storage.hpp>

#include <atomic>
#include <connection.hpp>

namespace ste {
namespace graphics {

class primary_renderer_buffers {
	friend class primary_renderer;

	using camera_t = camera<float, camera_projection_reversed_infinite_perspective>;

private:
	static gl::pipeline_external_binding_set_collection create_common_binding_set_collection(const ste_context &ctx,
																							 const renderer_transform_buffers &transform_buffers,
																							 const linked_light_lists &linked_light_list_storage,
																							 const deferred_gbuffer &gbuffer,
																							 const shadowmap_storage &shadows,
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
		gl::pipeline_external_binding_set_collection set{ 
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

		set["linked_light_list_size"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_size_map()));
		set["linked_light_list_low_detail_size"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_low_detail_size_map()));
		set["linked_light_list_heads"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_heads_map()));
		set["linked_light_list_low_detail_heads"] = gl::bind(gl::pipeline::storage_image(linked_light_list_storage.linked_light_lists_low_detail_heads_map()));
		set["linked_light_list_counter_binding"] = gl::bind(linked_light_list_storage.linked_light_lists_counter_buffer());
		set["linked_light_list_binding"] = gl::bind(linked_light_list_storage.linked_light_lists_buffer());

		set["shaped_lights_points_binding"] = gl::bind(s->properties().lights_storage().get_shaped_lights_points_buffer());

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

		return set;
	}
	static int gbuffer_depth_target_levels() {
		return static_cast<int>(glm::ceil(glm::log(linked_light_lists::lll_image_res_multiplier))) + 1;
	}

private:
	alias<const ste_context> ctx;

	connection<> gbuffer_depth_target_connection;
	connection<> shadows_storage_connection;

	glm::uvec2 extent;
	std::atomic_flag projection_data_up_to_date_flag;

	renderer_transform_buffers transform_buffers;
	atmospherics_buffer atmospheric_buffer;

	ste_resource<deferred_gbuffer> gbuffer;

	ste_resource<shadowmap_storage> shadows_storage;
	ste_resource<linked_light_lists> linked_light_list_storage;
	ste_resource<volumetric_scattering_storage> vol_scat_storage;

	mutable gl::pipeline_external_binding_set_collection common_binding_set_collection;

public:
	primary_renderer_buffers(const ste_context &ctx,
							 const glm::uvec2 &extent,
							 const scene *s,
							 const atmospherics_properties<double> &atmospherics_prop)
		: ctx(ctx),
		extent(extent),

		transform_buffers(ctx),
		atmospheric_buffer(ctx, atmospherics_prop),

		gbuffer(ctx, extent, gbuffer_depth_target_levels()),

		shadows_storage(ctx),
		linked_light_list_storage(ctx, extent),
		vol_scat_storage(ctx, extent),

		common_binding_set_collection(create_common_binding_set_collection(ctx,
																		   transform_buffers,
																		   *linked_light_list_storage,
																		   *gbuffer,
																		   *shadows_storage,
																		   s)) 
	{
		projection_data_up_to_date_flag.test_and_set(std::memory_order_release);

		// gbuffer resize signal
		gbuffer_depth_target_connection = make_connection(gbuffer->get_depth_target_modified_signal(), [this]() {
		});
		// Shadow storage change signal
		shadows_storage_connection = make_connection(shadows_storage->get_storage_modified_signal(), [&]() {
		});
	}
	~primary_renderer_buffers() noexcept {}

	/**
	 *	@brief		Resize
	 */
	void resize(const glm::uvec2 &extent) {
		if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
			return;

		this->extent = extent;

		// Resize buffers
		gbuffer->resize(extent);
		linked_light_list_storage->resize(extent);
		vol_scat_storage->resize(extent);

		// Set resized flag
		projection_data_up_to_date_flag.clear(std::memory_order_release);
	}

	/**
	*	@brief		Updates common descriptor set's bindings and data
	*/
	void update(gl::command_recorder &recorder,
				scene *s,
				const camera_t *cam) {
		// Update material bindings, if materials were mutated
		common_binding_set_collection["material_textures_count"] = s->properties().materials_storage().get_material_texture_storage().size();
		common_binding_set_collection["material_textures"] = s->properties().materials_storage().get_material_texture_storage().binder();

		// Upload new camera transform data
		transform_buffers.update_view_data(recorder, *cam);

		// If needed, upload new camera projection data (after resize)
		if (!projection_data_up_to_date_flag.test_and_set(std::memory_order_acquire)) {
			transform_buffers.update_proj_data(recorder, *cam, extent);
		}
	}

	/**
	 *	@brief		Sets new atmospheric properties
	 */
	void update_atmospheric_properties(gl::command_recorder &recorder,
									   const atmospherics_properties<double> &atmospherics_prop) {
		atmospheric_buffer.update_data(recorder,
									   atmospherics_prop);
	}

	auto& get_extent() const { return extent; }
};

}
}
