// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "material_layer_descriptor.hpp"
#include "observable_resource.hpp"

#include "Texture2D.hpp"
#include "Sampler.hpp"

#include <memory>

namespace StE {
namespace Graphics {

/**
 *	@brief	Defines rendering material layer
 */
class material_layer : public Core::observable_resource<material_layer_descriptor> {
	using Base = Core::observable_resource<material_layer_descriptor>;

private:
	Core::SamplerMipmapped material_sampler;

	std::shared_ptr<Core::Texture2D> basecolor_map{ nullptr };
	
	float thickness{ .0f };

	float roughness{ .5f };
	float anisotropy{ .0f };
	float metallic{ .0f };

	float index_of_refraction{ 1.5f };
	float absorption_alpha{ .0f };

	float sheen{ .0f };
	float sheen_power{ .0f };

	material_layer *next_layer{ nullptr };

private:
	material_layer_descriptor descriptor;

private:
	material_layer(material_layer &&m) = delete;
	material_layer(const material_layer &m) = delete;
	material_layer &operator=(material_layer &&m) = delete;
	material_layer &operator=(const material_layer &m) = delete;

private:
	Core::texture_handle handle_for_texture(const Core::Texture2D *t) const;

public:
	/**
	*	@brief	Convert material anisotropy value to ratio which is used to adjust anisotropic roughness values
	*/
	static float convert_anisotropy_to_ratio(float ansio) {
		return ansio != .0f ? glm::sqrt(1.f - ansio * .9f) : 1.f;
	}

public:
	material_layer();
	~material_layer() {
		basecolor_map = nullptr;
	}

	/**
	*	@brief	Set material base color (diffuse) map
	*
	* 	@param tex	2D texture object
	*/
	void set_basecolor_map(const std::shared_ptr<Core::Texture2D> &tex) {
		basecolor_map = tex;
		descriptor.basecolor_handle = handle_for_texture(basecolor_map.get());
		Base::notify();
	}

	/**
	*	@brief	Set material roughness
	*
	*	Roughness as defines by the Micorfacet theory. Controls both diffuse and specular response. Defaults to 0.5.
	*
	* 	@param r	Roughness - range: [0,1]
	*/
	void set_roughness(float r) {
		roughness = r;
		descriptor.set_roughness_and_thickness(roughness, thickness);
		Base::notify();
	}

	/**
	*	@brief	Set material anisotropy
	*
	*	Anisotropy modifies material's anisotropic roughness
	*
	* 	@param a	Anisotropy - range: [0,1] (May take negative values which invert X, Y anisotropy)
	*/
	void set_anisotropy(float a) {
		anisotropy = a;
		descriptor.set_anisotropy_and_metallicity(convert_anisotropy_to_ratio(anisotropy), metallic);
		Base::notify();
	}

	/**
	*	@brief	Set material metallicity
	*
	*	Controls material's metal appearance. Defaults to 0.0.
	*
	* 	@param m	Metallicity - range: [0,1] (Usually a binary value)
	*/
	void set_metallic(float m) {
		metallic = m;
		descriptor.set_anisotropy_and_metallicity(convert_anisotropy_to_ratio(anisotropy), metallic);
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
		index_of_refraction = glm::max(1.f, ior);
		descriptor.ior = index_of_refraction;
		Base::notify();
	}

	/**
	*	@brief	Set material absorption coefficient
	*
	*	Sets the absorption coefficient (alpha) as per the Beer–Lambert law. 
	*	Absorption is wave-length dependent, with the dependence being the inverse of the luminance of the material base color
	*	multiplied by alpha. Defaults to 0.
	*
	* 	@param a	Absorption alpha - range: [0,infinity)
	*/
	void set_absorption_alpha(float a) {
		absorption_alpha = glm::max(.0f, a);
		descriptor.absorption_alpha = absorption_alpha;
		Base::notify();
	}

	/**
	*	@brief	Set material sheen
	*
	*	Sheen provides an additional cloth-like grazing component. Defaults to 0.0.
	*	Similiar to Disney's implementation.
	*
	* 	@param s	Sheen value	- range: [0,1]
	*/
	void set_sheen(float s) {
		sheen = s;
		descriptor.set_sheen(sheen, sheen_power);
		Base::notify();
	}

	/**
	*	@brief	Set material sheen power
	*
	*	Controls sheen's curve. Defaults to 0.0.
	*
	* 	@param sp	Sheen power value	- range: [0,1]
	*/
	void set_sheen_power(float sp) {
		sheen_power = sp;
		descriptor.set_sheen(sheen, sheen_power);
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
		thickness = t;
		descriptor.set_roughness_and_thickness(roughness, thickness);
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

		descriptor.next_layer_id = layerid;
		next_layer = layerid == material_layer_none ? nullptr : layer;
		
		Base::notify();
	}

	auto *get_basecolor_map() const { return basecolor_map.get(); }
	float get_roughness() const { return roughness; }
	float get_anisotropy() const { return anisotropy; }
	float get_metallic() const { return metallic; }
	float get_index_of_refraction() const { return index_of_refraction; }
	float get_sheen() const { return sheen; }
	float get_sheen_power() const { return sheen_power; }
	float get_layer_thickness() const { return thickness; }
	float get_absorption_alpha() const { return absorption_alpha; }

	auto *get_next_layer() const { return next_layer; }

	const material_layer_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
