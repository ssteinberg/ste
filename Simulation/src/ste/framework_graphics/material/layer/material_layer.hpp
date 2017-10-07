//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_layer_descriptor.hpp>
#include <observable_resource.hpp>

#include <material_textures_storage.hpp>
#include <material_texture.hpp>

#include <command_recorder.hpp>
#include <cmd_clear_color_image.hpp>

#include <rgb.hpp>
#include <alias.hpp>
#include <limits>
#include <optional.hpp>
#include <mutex>
#include <atomic>
#include <lib/aligned_padded_ptr.hpp>

namespace ste {
namespace graphics {

/**
 *	@brief	Defines rendering material layer
 *			Thread-safe
 */
class material_layer : public gl::observable_resource<material_layer_descriptor> {
	using Base = gl::observable_resource<material_layer_descriptor>;

private:
	static constexpr auto layer_maps_shader_stage_access = gl::pipeline_stage::fragment_shader;

	struct update_data_t {
		mutable std::mutex m;
		mutable std::atomic<bool> update_flag{ false };

		bool has_mutable_map;
		float new_value;
	};

private:
	alias<const ste_context> ctx;
	alias<material_textures_storage> textures_storage;

	rgb albedo;
	
	// Maps
	material_texture roughness_map;
	material_texture metallicity_map;
	material_texture thickness_map;
	//material_texture anisotropy_map;

	// Maps' update data
	lib::aligned_padded_ptr<update_data_t> roughness_map_update_data;
	lib::aligned_padded_ptr<update_data_t> metallicity_map_update_data;
	lib::aligned_padded_ptr<update_data_t> thickness_map_update_data;
//	lib::aligned_padded_ptr<update_data_t> anisotropy_map_update_data;

	std::mutex ior_phase_mutex;
	std::mutex attenuation_mutex;
	std::mutex next_layer_mutex;

	float index_of_refraction{ 1.5f };
	glm::vec3 attenuation_coefficient{ std::numeric_limits<float>::infinity() };
	float phase_g{ .0f };

	material_layer *next_layer{ nullptr };

private:
	material_layer_descriptor descriptor;

private:
	material_layer(material_layer &&m) = delete;
	material_layer(const material_layer &m) = delete;
	material_layer &operator=(material_layer &&m) = delete;
	material_layer &operator=(const material_layer &m) = delete;

private:
	/**
	 *	@brief	Assigns default values to roughness, metallicity, anisotropy and thickness maps.
	 */
	void set_default_maps();

	static void update_resource_map(gl::command_recorder &recorder,
									const update_data_t &data,
									const gl::texture<gl::image_type::image_2d> &texture) {
		// Record map update
		recorder
			<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(layer_maps_shader_stage_access,
															 gl::pipeline_stage::transfer,
															 gl::image_memory_barrier(texture.get_image(),
																					  gl::image_layout::undefined,
																					  gl::image_layout::transfer_dst_optimal,
																					  gl::access_flags::shader_read,
																					  gl::access_flags::transfer_write)))
			<< gl::cmd_clear_color_image(texture.get_image(),
										 gl::image_layout::transfer_dst_optimal,
										 glm::vec4{ data.new_value })
			<< gl::cmd_pipeline_barrier(gl::pipeline_barrier(gl::pipeline_stage::transfer,
															 layer_maps_shader_stage_access,
															 gl::image_memory_barrier(texture.get_image(),
																					  gl::image_layout::transfer_dst_optimal,
																					  gl::image_layout::shader_read_only_optimal,
																					  gl::access_flags::transfer_write,
																					  gl::access_flags::shader_read)));
	}

public:
	/**
	*	@brief	Convert material anisotropy value to ratio which is used to adjust anisotropic roughness values
	*/
	static float convert_anisotropy_to_ratio(float ansio) {
		float ratio = glm::sqrt(1.f - glm::abs(ansio) * material_layer_ansio_ratio_scale);
		if (ansio < .0f)
			ratio = 1.f / ratio;
		return ratio;
	}

protected:
	void update_resource(gl::command_recorder &recorder) const override final {
		// Check if there is anything to do
		const bool has_roughness_updates = roughness_map_update_data->update_flag.exchange(false, std::memory_order_acq_rel);
		const bool has_metallicity_updates = metallicity_map_update_data->update_flag.exchange(false, std::memory_order_acq_rel);
		const bool has_thickness_updates = thickness_map_update_data->update_flag.exchange(false, std::memory_order_acq_rel);
//		const bool has_anisotropy_updates = anisotropy_map_update_data->update_flag.exchange(false, std::memory_order_acq_rel);
		const bool has_updates =
			has_roughness_updates ||
			has_metallicity_updates ||
			has_thickness_updates;
//			has_anisotropy_updates;

		if (!has_updates)
			return;

		// Update resources' maps
		if (has_roughness_updates) {
			std::unique_lock<std::mutex> l(roughness_map_update_data->m);
			update_resource_map(recorder,
								*roughness_map_update_data,
								roughness_map.texture());
		}
		if (has_metallicity_updates) {
			std::unique_lock<std::mutex> l(metallicity_map_update_data->m);
			update_resource_map(recorder,
								*metallicity_map_update_data,
								metallicity_map.texture());
		}
		if (has_thickness_updates) {
			std::unique_lock<std::mutex> l(thickness_map_update_data->m);
			update_resource_map(recorder,
								*thickness_map_update_data,
								thickness_map.texture());
		}
//		if (has_anisotropy_updates) {
//			std::unique_lock<std::mutex> l(anisotropy_map_update_data->m);
//			update_resource_map(recorder,
//								*anisotropy_map_update_data,
//								anisotropy_map.texture());
//		}
	}

public:
	material_layer(const ste_context &ctx,
				   material_textures_storage &textures_storage);
	~material_layer() noexcept {}

	/**
	*	@brief	Set material albedo
	*
	*	Albedo of the material, defaults to { 0,0,0 }.
	*
	* 	@param rgb	Material albedo
	*/
	void set_albedo(const rgb &rgb) {
		albedo = rgb;

		const glm::vec3 v = rgb;
		descriptor.set_albedo(glm::vec4{ v.r, v.g, v.b, 1.f });
		Base::notify();
	}

	/**
	*	@brief	Set material roughness
	*
	*	Roughness as defines by the microfacet theory. Controls both diffuse and specular response. Defaults to 0.5.
	*
	* 	@param r	Roughness - range: [0,1]
	*/
	void set_roughness(float r) {
		// If needed, replace the default blank map with a custom uninitialized one.
		{
			std::unique_lock<std::mutex> l(roughness_map_update_data->m);

			if (roughness_map_update_data->has_mutable_map) {
				// If still using the default map, create a new one.
				roughness_map = textures_storage->allocate_uninitialized_texture(ctx);
				descriptor.set_roughness_map_handle(roughness_map.texture_index());

				roughness_map_update_data->has_mutable_map = false;
			}

			roughness_map_update_data->new_value = r;
			roughness_map_update_data->update_flag.store(true, std::memory_order_release);
		}

		Base::notify();
	}
	/**
	*	@brief	Set material roughness map
	*
	*	Roughness as defines by the microfacet theory. Controls both diffuse and specular response. Defaults to 0.5.
	*
	* 	@param map	Roughness map
	*/
	void set_roughness(const material_texture &map) {
		{
			std::unique_lock<std::mutex> l(roughness_map_update_data->m);

			roughness_map = map;
			descriptor.set_roughness_map_handle(roughness_map.texture_index());

			roughness_map_update_data->has_mutable_map = false;
		}

		Base::notify();
	}

	/**
	*	@brief	Set material anisotropy
	*
	*	Anisotropy modifies material's anisotropic roughness
	*
	* 	@param a	Anisotropy - range: [0,1] (May take negative values which invert X, Y anisotropy)
	*/
//	void set_anisotropy(float a) {
//		// If needed, replace the default blank map with a custom uninitialized one.
//		{
//			std::unique_lock<std::mutex> l(anisotropy_map_update_data->m);
//
//			if (anisotropy_map_update_data->has_mutable_map) {
//				// If still using the default map, create a new one.
//				anisotropy_map = textures_storage->allocate_uninitialized_texture(ctx);
//				descriptor.set_anisotropy_map_handle(anisotropy_map.texture_index());
//
//				anisotropy_map_update_data->has_mutable_map = false;
//			}
//
//			anisotropy_map_update_data->new_value = a;
//			anisotropy_map_update_data->update_flag.store(true, std::memory_order_release);
//		}
//
//		Base::notify();
//	}
	/**
	*	@brief	Set material anisotropy map
	*
	*	Anisotropy modifies material's anisotropic roughness
	*
	* 	@param map	Anisotropy map
	*/
//	void set_anisotropy(const material_texture &map) {
//		{
//			std::unique_lock<std::mutex> l(anisotropy_map_update_data->m);
//
//			anisotropy_map = map;
//			descriptor.set_anisotropy_map_handle(anisotropy_map.texture_index());
//
//			anisotropy_map_update_data->has_mutable_map = false;
//		}
//
//		Base::notify();
//	}

/**
*	@brief	Set material metallicity
*
*	Controls material's metal appearance. Defaults to 0.0.
*
* 	@param m	Metallicity - range: [0,1]
*/
	void set_metallicity(float m) {
		// If needed, replace the default blank map with a custom uninitialized one.
		{
			std::unique_lock<std::mutex> l(metallicity_map_update_data->m);

			if (metallicity_map_update_data->has_mutable_map) {
				// If still using the default map, create a new one.
				metallicity_map = textures_storage->allocate_uninitialized_texture(ctx);
				descriptor.set_metallicity_map_handle(metallicity_map.texture_index());

				metallicity_map_update_data->has_mutable_map = false;
			}

			metallicity_map_update_data->new_value = m;
			metallicity_map_update_data->update_flag.store(true, std::memory_order_release);
		}

		Base::notify();
	}
	/**
	*	@brief	Set material metallicity map
	*
	*	Controls material's metal appearance. Defaults to 0.0.
	*
	* 	@param map	Metallicity map
	*/
	void set_metallicity(const material_texture &map) {
		{
			std::unique_lock<std::mutex> l(metallicity_map_update_data->m);

			metallicity_map = map;
			descriptor.set_metallicity_map_handle(metallicity_map.texture_index());

			metallicity_map_update_data->has_mutable_map = false;
		}

		Base::notify();
	}

	/**
	*	@brief	Set material incident specular amount given an index-of-refraction
	*
	*	Sets specular term using a given index-of-refraction. Defaults to 1.5.
	*
	* 	@param ior	Index-of-refraction - range: [1,infinity) (Usually in range [1,2] for non-metals)
	*/
	void set_index_of_refraction(float ior) {
		{
			std::unique_lock<std::mutex> l(ior_phase_mutex);

			index_of_refraction = ior;
			descriptor.set_ior_phase(index_of_refraction, phase_g);
		}

		Base::notify();
	}

	/**
	*	@brief	Set material attenuation coefficient
	*
	*	Sets the total attenuation coefficient as per the Beer–Lambert law.
	*	Total attenuation equals to scattering + absorption, both are wavelength dependent.
	*	Scattering is dependent according to the material layer albedo. The rest is absorbed light.
	*
	*	Defaults to { 0, 0, 0 }
	*
	* 	@param a	Per-channel total attenuation coefficient  - range: [0,infinity)
	*/
	void set_attenuation_coefficient(const glm::vec3 a) {
		{
			std::unique_lock<std::mutex> l(attenuation_mutex);

			attenuation_coefficient = a;
			descriptor.set_attenuation_coefficient(attenuation_coefficient);
		}

		Base::notify();
	}

	/**
	*	@brief	Set material Henyey-Greenstein phase function g parameter
	*
	*	Controls the subsurface-scattering phase function. The parameter adjusts the relative amount of
	*	back and forward scattering with a value of 0 corresponding to purely isotropic scattering, values 
	*	close to -1 give highly peaked back scattering and values close to +1 give highly peaked forward 
	*	scattering.
	*
	*	Defaults to 0
	*
	* 	@param g	Henyey-Greenstein phase function parameter  - range: (-1,+1)
	*/
	void set_scattering_phase_parameter(float g) {
		{
			std::unique_lock<std::mutex> l(ior_phase_mutex);

			phase_g = g;
			descriptor.set_ior_phase(index_of_refraction, phase_g);
		}

		Base::notify();
	}

	/**
	*	@brief	Set material layer thickness
	*
	*	Controls the material layer thickness. Ignored for base layers.
	*
	* 	@param t	Thickness in standard units	- range: (0,material_layer_max_thickness)
	*/
	void set_layer_thickness(float t) {
		// If needed, replace the default blank map with a custom uninitialized one.
		{
			std::unique_lock<std::mutex> l(thickness_map_update_data->m);

			if (thickness_map_update_data->has_mutable_map) {
				// If still using the default map, create a new one.
				thickness_map = textures_storage->allocate_uninitialized_texture(ctx);
				descriptor.set_thickness_map_handle(thickness_map.texture_index());

				thickness_map_update_data->has_mutable_map = false;
			}

			thickness_map_update_data->new_value = t;
			thickness_map_update_data->update_flag.store(true, std::memory_order_release);
		}

		Base::notify();
	}
	/**
	*	@brief	Set material layer thickness map
	*
	*	Controls the material layer thickness. Ignored for base layers.
	*
	* 	@param map	Thickness map in standard units	- range: (0,material_layer_max_thickness)
	*/
	void set_layer_thickness(const material_texture &map) {
		{
			std::unique_lock<std::mutex> l(thickness_map_update_data->m);

			thickness_map = map;
			descriptor.set_thickness_map_handle(thickness_map.texture_index());

			thickness_map_update_data->has_mutable_map = false;
		}

		Base::notify();
	}
	
	/**
	*	@brief	Set next layer
	*
	*	Layers are stacked top-to-bottom as a single linked list. This method sets the next layer id (the layer directly under the current layer),
	*	or nullptr if this is the base material.
	*
	* 	@param layer	Material layer
	*/
	void set_next_layer(material_layer *layer) {
		int layerid = material_layer_none;
		if (layer != nullptr) {
			auto id = layer->resource_index_in_storage();
			assert(id >= 0);
			if (id >= 0)
				layerid = id;
		}

		{
			std::unique_lock<std::mutex> l(next_layer_mutex);

			descriptor.set_next_layer_id(layerid);
			next_layer = layerid == material_layer_none ? nullptr : layer;
		}
		
		Base::notify();
	}

	auto get_albedo() const { return albedo; }
	auto get_index_of_refraction() const { return index_of_refraction; }
	auto get_attenuation_coefficient() const { return attenuation_coefficient; }
	float get_scattering_phase_parameter() const { return phase_g; }

	auto& get_roughness_map() const { return roughness_map; }
	auto& get_metallicity_map() const { return metallicity_map; }
	auto& get_thickness_map() const { return thickness_map; }
//	auto& get_anisotropy_map() const { return anisotropy_map; }

	auto *get_next_layer() const { return next_layer; }

	material_layer_descriptor get_descriptor() const override final {
		return descriptor;
	}
};

}
}
