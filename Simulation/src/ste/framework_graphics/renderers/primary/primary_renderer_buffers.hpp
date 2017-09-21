// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <rendering_system.hpp>

#include <pipeline_external_binding_set.hpp>
#include <scene.hpp>

#include <deferred_gbuffer.hpp>
#include <atmospherics_buffer.hpp>
#include <atmospherics_lut_storage.hpp>
#include <renderer_transform_buffers.hpp>
#include <linked_light_lists.hpp>

#include <atomic>
#include <connection.hpp>

namespace ste {
namespace graphics {

class primary_renderer_buffers {
	friend class primary_renderer;

	using camera_t = camera<float, camera_projection_reversed_infinite_perspective>;

private:
	static int gbuffer_depth_target_levels() {
		return static_cast<int>(glm::ceil(glm::log(linked_light_lists::lll_image_res_multiplier))) + 1;
	}

	static gl::pipeline_external_binding_set create_common_binding_set_collection(const ste_context &ctx);

private:
	alias<const ste_context> ctx;
	const scene *s;

	connection<> gbuffer_depth_target_connection;
	connection<> lll_storage_connection;

	glm::uvec2 extent;
	std::atomic_flag projection_data_up_to_date_flag;

	ste_resource<linked_light_lists> linked_light_list_storage;
	gl::rendering_system::storage_ptr<atmospherics_lut_storage> atmospherics_luts;
	atmospherics_buffer atmospheric_buffer;

	ste_resource<deferred_gbuffer> gbuffer;
	renderer_transform_buffers transform_buffers;

	mutable gl::pipeline_external_binding_set common_binding_set_collection;

private:
	void common_binding_set_bind_transform_buffers();
	void common_binding_set_bind_mesh_and_materials();
	void common_binding_set_bind_light_buffers();
	void common_binding_set_bind_lll_buffers();
	void common_binding_set_bind_gbuffer();
	void common_binding_set_bind_atmospheric_buffers();

public:
	primary_renderer_buffers(const ste_context &ctx,
							 const glm::uvec2 &extent,
							 const scene *s,
							 gl::rendering_system::storage_ptr<atmospherics_lut_storage> &&atmospherics_luts,
							 const atmospherics_properties<double> &atmospherics_prop)
		: ctx(ctx),
		s(s),
		extent(extent),

		linked_light_list_storage(ctx, extent),
		atmospherics_luts(std::move(atmospherics_luts)),
		atmospheric_buffer(ctx, atmospherics_prop),

		gbuffer(ctx, extent, gbuffer_depth_target_levels()),
		transform_buffers(ctx),

		common_binding_set_collection(create_common_binding_set_collection(ctx))
	{
		projection_data_up_to_date_flag.clear(std::memory_order_release);

		// Bind common binding set resources
		common_binding_set_bind_transform_buffers();
		common_binding_set_bind_mesh_and_materials();
		common_binding_set_bind_light_buffers();
		common_binding_set_bind_lll_buffers();
		common_binding_set_bind_gbuffer();
		common_binding_set_bind_atmospheric_buffers();

		// gbuffer resize signal
		gbuffer_depth_target_connection = make_connection(gbuffer->get_gbuffer_modified_signal(), [this]() {
			common_binding_set_bind_gbuffer();
		});
		// Linked-light-lists storage change signal
		lll_storage_connection = make_connection(linked_light_list_storage->get_storage_modified_signal(), [&]() {
			common_binding_set_bind_lll_buffers();
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

		// Set resized flag
		projection_data_up_to_date_flag.clear(std::memory_order_release);
	}

	/**
	 *	@brief		Should be called on camera projection change
	 */
	void invalidate_projection_buffer() {
		projection_data_up_to_date_flag.clear(std::memory_order_release);
	}

	/**
	*	@brief		Update the common binding set
	*/
	void update_common_binding_set(scene *s);

	/**
	*	@brief		Updates common descriptor set's bindings and data
	*/
	void update(gl::command_recorder &recorder,
				const camera_t *cam);

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
