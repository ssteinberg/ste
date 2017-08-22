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
#include <transforms_ring_buffers.hpp>
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
																							 const scene *s) {
		// Load common descriptor sets from pre-compiled SPIR-v shader
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
	static int gbuffer_depth_target_levels() {
		return glm::ceil(glm::log(linked_light_lists::lll_image_res_multiplier)) + 1;
	}

private:
	std::reference_wrapper<const ste_context> ctx;

	connection<> gbuffer_depth_target_connection;

	glm::ivec2 extent;
	std::atomic_flag projection_data_up_to_date_flag;

	transforms_ring_buffers transform_buffers;
	atmospherics_buffer atmospheric_buffer;

	deferred_gbuffer gbuffer;

	ste_resource<linked_light_lists> lll_storage;
	ste_resource<shadowmap_storage> shadows_storage;
	ste_resource<volumetric_scattering_storage> vol_scat_storage;

	gl::pipeline_external_binding_set_collection common_binding_set_collection;

public:
	primary_renderer_buffers(const ste_context &ctx,
							 const glm::ivec2 &extent,
							 const scene *s,
							 const atmospherics_properties<double> &atmospherics_prop)
		: ctx(ctx),
		extent(extent),

		transform_buffers(ctx),
		atmospheric_buffer(ctx, atmospherics_prop),

		gbuffer(ctx, extent, gbuffer_depth_target_levels()),

		lll_storage(ctx, extent),
		shadows_storage(ctx),
		vol_scat_storage(ctx),

		common_binding_set_collection(create_common_binding_set_collection(ctx,
																		   s)) 
	{
		projection_data_up_to_date_flag.test_and_set(std::memory_order_release);

		// Attach gbuffer's depth map to the scatter fragment
		vol_scat_storage->set_depth_maps(gbuffer.get_depth_target(), gbuffer.get_downsampled_depth_target());
		gbuffer_depth_target_connection = make_connection(gbuffer->get_depth_target_modified_signal(), [this]() {
			vol_scat_storage->set_depth_maps(gbuffer.get_depth_target(), gbuffer.get_downsampled_depth_target());
		});
	}
	~primary_renderer_buffers() noexcept {}

	/**
	 *	@brief		Resize
	 */
	void resize(const glm::ivec2 &extent) {
		if (extent.x <= 0 || extent.y <= 0 || extent == this->extent)
			return;

		this->extent = extent;

		// Resize buffers
		gbuffer.resize(extent);
		lll_storage->resize(extent);
		vol_scat_storage.resize(extent);

		// Set resized flag
		projection_data_up_to_date_flag.clear(std::memory_order_release);
	}

	/**
	*	@brief		Updates common descriptor set's bindings and data
	*/
	void update(gl::command_recorder &recorder,
				const scene *s,
				const camera_t *cam) {
		// Update material bindings, in materials were mutated
		common_binding_set_collection["material_samplers"] = s->properties().materials_storage().get_material_texture_storage().binder();

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
