// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "material_layer_descriptor.hpp"
#include "observable_resource.hpp"

#include "Texture2D.hpp"
#include "Sampler.hpp"

#include "RGB.hpp"

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

	RGB color;
	
	float thickness{ .0f };

	float roughness{ .5f };
	float anisotropy{ .0f };
	float aniso_ratio{ 1.f };
	float metallic{ .0f };

	float index_of_refraction{ 1.5f };
	glm::vec3 attenuation_coefficient{ .0f };
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
	Core::texture_handle handle_for_texture(const Core::Texture2D *t) const;

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
	*	@brief	Set material color
	*
	*	Color of the material, defaults to { 0,0,0 }.
	*
	* 	@param rgb	Material color
	*/
	void set_color(const RGB &rgb) {
		color = rgb;

		glm::vec3 v = rgb;
		descriptor.set_color(glm::vec4{ v.r, v.g, v.b, 1.f });
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
		aniso_ratio = convert_anisotropy_to_ratio(anisotropy);
		descriptor.set_anisotropy_and_metallicity(aniso_ratio, metallic);
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
		descriptor.set_anisotropy_and_metallicity(aniso_ratio, metallic);
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
		descriptor.set_ior(index_of_refraction);
		Base::notify();
	}

	/**
	*	@brief	Set material attenuation coefficient
	*
	*	Sets the total attenuation coefficient as per the Beer–Lambert law.
	*	Total attenuation equals to scattering + absorption, both are wavelength dependant.
	*	Scattering is dependent according to the material layer color. The rest is absorped light.
	*
	*	Defaults to { 0, 0, 0 }
	*
	* 	@param a	Total attenuation coefficient  - range: [0,infinity)
	*/
	void set_attenuation_coefficient(const glm::vec3 a) {
		attenuation_coefficient = a;
		descriptor.set_attenuation_coefficient(attenuation_coefficient, phase_g);
		Base::notify();
	}

	/**
	*	@brief	Set material Henyey-Greenstein phase function g parameter
	*
	*	Controls the subsurface-scattering pahse function. The parameter adjusts the relative amount of
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
		descriptor.set_attenuation_coefficient(attenuation_coefficient, phase_g);
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

		descriptor.set_next_layer_id(layerid);
		next_layer = layerid == material_layer_none ? nullptr : layer;
		
		Base::notify();
	}

	auto get_color() const { return color; }
	float get_roughness() const { return roughness; }
	float get_anisotropy() const { return anisotropy; }
	float get_metallic() const { return metallic; }
	float get_index_of_refraction() const { return index_of_refraction; }
	float get_layer_thickness() const { return thickness; }
	auto get_attenuation_coefficient() const { return attenuation_coefficient; }
	float get_scattering_phase_parameter() const { return phase_g; }

	auto *get_next_layer() const { return next_layer; }

	const material_layer_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
