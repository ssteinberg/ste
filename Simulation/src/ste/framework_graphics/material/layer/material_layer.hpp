// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "material_layer_descriptor.hpp"
#include "observable_resource.hpp"

#include "texture_2d.hpp"
#include "Sampler.hpp"

#include "rgb.hpp"

#include <memory>
#include <limits>

namespace StE {
namespace Graphics {

/**
 *	@brief	Defines rendering material layer
 */
class material_layer : public Core::observable_resource<material_layer_descriptor> {
	using Base = Core::observable_resource<material_layer_descriptor>;

private:
	Core::sampler_mipmapped material_sampler;

	rgb albedo;
	
	std::shared_ptr<Core::texture_2d> roughness_map{ nullptr };
	std::shared_ptr<Core::texture_2d> metallicity_map{ nullptr };
	std::shared_ptr<Core::texture_2d> thickness_map{ nullptr };
	//std::shared_ptr<Core::texture_2d> anisotropy_map{ nullptr };

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
	Core::texture_handle handle_for_texture(const Core::texture_2d *t) const;

	template <std::shared_ptr<Core::texture_2d> material_layer::*map>
	void write_scalar_map(float scalar) {
		if (this->*map == nullptr || (this->*map)->get_size() != glm::ivec2{ 1, 1 }) {
			auto surface = gli::texture2d(gli::format::FORMAT_R32_SFLOAT_PACK32, { 1, 1 }, 1);
			*reinterpret_cast<float*>(surface.data()) = scalar;
			this->*map = std::make_shared<StE::Core::texture_2d>(surface, false);
		}
		else
			(this->*map)->clear(&scalar);
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

public:
	material_layer();

	/**
	*	@brief	Set material albedo
	*
	*	Albedo of the material, defaults to { 0,0,0 }.
	*
	* 	@param rgb	Material albedo
	*/
	void set_albedo(const rgb &rgb) {
		albedo = rgb;

		glm::vec3 v = rgb;
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
		write_scalar_map<&material_layer::roughness_map>(r);
		descriptor.set_roughness_map_handle(handle_for_texture(roughness_map.get()));

		Base::notify();
	}
	/**
	*	@brief	Set material roughness map
	*
	*	Roughness as defines by the microfacet theory. Controls both diffuse and specular response. Defaults to 0.5.
	*
	* 	@param map	Roughness map
	*/
	void set_roughness(const std::shared_ptr<Core::texture_2d> &map) {
		this->roughness_map = map;
		descriptor.set_roughness_map_handle(handle_for_texture(roughness_map.get()));

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
//		write_scalar_map<&material_layer::anisotropy_map>(a);
//		descriptor.set_anisotropy_map_handle(handle_for_texture(anisotropy_map.get()));
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
//	void set_anisotropy(const std::shared_ptr<Core::texture_2d> &map) {
//		this->anisotropy_map = map;
//		descriptor.set_anisotropy_map_handle(handle_for_texture(anisotropy_map.get()));
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
	void set_metallic(float m) {
		write_scalar_map<&material_layer::metallicity_map>(m);
		descriptor.set_metallicity_map_handle(handle_for_texture(metallicity_map.get()));

		Base::notify();
	}
	/**
	*	@brief	Set material metallicity map
	*
	*	Controls material's metal appearance. Defaults to 0.0.
	*
	* 	@param map	Metallicity map
	*/
	void set_metallic(const std::shared_ptr<Core::texture_2d> &map) {
		metallicity_map = map;
		descriptor.set_metallicity_map_handle(handle_for_texture(metallicity_map.get()));
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
		write_scalar_map<&material_layer::thickness_map>(t);
		descriptor.set_thickness_map_handle(handle_for_texture(thickness_map.get()));

		Base::notify();
	}
	/**
	*	@brief	Set material layer thickness map
	*
	*	Controls the material layer thickness. Ignored for base layers.
	*
	* 	@param map	Thickness map in standard units	- range: (0,material_layer_max_thickness)
	*/
	void set_layer_thickness(const std::shared_ptr<Core::texture_2d> &map) {
		thickness_map = map;
		descriptor.set_thickness_map_handle(handle_for_texture(thickness_map.get()));
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

	auto *get_next_layer() const { return next_layer; }

	const material_layer_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
