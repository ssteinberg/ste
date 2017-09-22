//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_layer_descriptor.hpp>
#include <observable_resource.hpp>

#include <material_textures_storage.hpp>
#include <material_texture.hpp>

#include <texture.hpp>
#include <sampler.hpp>
#include <surface.hpp>
#include <surface_factory.hpp>

#include <rgb.hpp>
#include <alias.hpp>
#include <limits>

namespace ste {
namespace graphics {

/**
 *	@brief	Defines rendering material layer
 */
class material_layer : public gl::observable_resource<material_layer_descriptor> {
	using Base = gl::observable_resource<material_layer_descriptor>;

private:
	alias<const ste_context> ctx;
	alias<material_textures_storage> textures_storage;

	rgb albedo;
	
	material_texture roughness_map;
	material_texture metallicity_map;
	material_texture thickness_map;
	//material_texture anisotropy_map;

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
	auto create_scalar_map(float scalar) {
		auto surface = resource::surface_2d<gl::format::r32_sfloat>({ 1, 1 }, 1);
		surface[0][0].r() = scalar;
	
		auto image = resource::surface_factory::image_from_surface_2d<gl::format::r32_sfloat>(ctx,
																							  std::move(surface),
																							  gl::image_usage::sampled,
																							  gl::image_layout::shader_read_only_optimal,
																							  "material_layer scalar_map",
																							  false);
		return gl::texture<gl::image_type::image_2d>(std::move(image).get());
	}

	/**
	 *	@brief	Assigns default values to roughness, metallicity, anisotropy and thickness maps.
	 */
	void set_default_maps();

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
		roughness_map = textures_storage->allocate_texture(create_scalar_map(r));
		descriptor.set_roughness_map_handle(roughness_map.texture_index());

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
		roughness_map = map;
		descriptor.set_roughness_map_handle(roughness_map.texture_index());

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
//		anisotropy_map = textures_storage->allocate_texture(create_scalar_map(a));
//		descriptor.set_anisotropy_map_handle(anisotropy_map.texture_index());
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
//		anisotropy_map = map;
//		descriptor.set_anisotropy_map_handle(anisotropy_map.texture_index());
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
		metallicity_map = textures_storage->allocate_texture(create_scalar_map(m));
		descriptor.set_metallicity_map_handle(metallicity_map.texture_index());

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
		metallicity_map = map;
		descriptor.set_metallicity_map_handle(metallicity_map.texture_index());
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
		index_of_refraction = ior;
		descriptor.set_ior_phase(index_of_refraction, phase_g);
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
		attenuation_coefficient = a;
		descriptor.set_attenuation_coefficient(attenuation_coefficient);
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
		phase_g = g;
		descriptor.set_ior_phase(index_of_refraction, phase_g);
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
		thickness_map = textures_storage->allocate_texture(create_scalar_map(t));
		descriptor.set_thickness_map_handle(thickness_map.texture_index());

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
		thickness_map = map;
		descriptor.set_thickness_map_handle(thickness_map.texture_index());
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

		descriptor.set_next_layer_id(layerid);
		next_layer = layerid == material_layer_none ? nullptr : layer;
		
		Base::notify();
	}

	auto get_albedo() const { return albedo; }
	auto get_index_of_refraction() const { return index_of_refraction; }
	auto get_attenuation_coefficient() const { return attenuation_coefficient; }
	float get_scattering_phase_parameter() const { return phase_g; }

	auto& get_roughness_map() const { return roughness_map; }
	auto& get_metallicity_map() const { return metallicity_map; }
	auto& get_thickness_map() const { return thickness_map; }

	auto *get_next_layer() const { return next_layer; }

	material_layer_descriptor get_descriptor() const override final {
		return descriptor;
	}
};

}
}
