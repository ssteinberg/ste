// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <pipeline_external_binding_set.hpp>
#include <scene.hpp>

#include <deferred_gbuffer.hpp>
#include <atmospherics_buffer.hpp>
#include <atmospherics_lut_storage.hpp>
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
	static int gbuffer_depth_target_levels() {
		return static_cast<int>(glm::ceil(glm::log(linked_light_lists::lll_image_res_multiplier))) + 1;
	}

	static gl::pipeline_external_binding_set create_common_binding_set_collection(const ste_context &ctx,
																				  const renderer_transform_buffers &transform_buffers,
																				  const linked_light_lists &linked_light_list_storage,
																				  const deferred_gbuffer &gbuffer,
																				  const shadowmap_storage &shadows,
																				  const atmospherics_lut_storage &atmospherics_luts,
																				  const scene *s);

	/**
	*	@brief		Update the common binding set
	*/
	void update_common_binding_set();

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

	mutable gl::pipeline_external_binding_set common_binding_set_collection;

public:
	primary_renderer_buffers(const ste_context &ctx,
							 const glm::uvec2 &extent,
							 const scene *s,
							 const atmospherics_lut_storage &atmospherics_luts,
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
																		   atmospherics_luts,
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
	 *	@brief		Should be called on camera projection change
	 */
	void invalidate_projection_buffer() {
		projection_data_up_to_date_flag.clear(std::memory_order_release);
	}

	/**
	*	@brief		Updates common descriptor set's bindings and data
	*/
	void update(gl::command_recorder &recorder,
				scene *s,
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
