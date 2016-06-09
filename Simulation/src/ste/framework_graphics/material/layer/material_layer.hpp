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
#include <functional>

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
	float anisotropy{ .0f };

	material_layer *next_layer;

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
		descriptor.roughness = r;
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
		descriptor.anisotropy_ratio = convert_anisotropy_to_ratio(a);
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
		descriptor.metallic = m;
		Base::notify();
	}

	/**
	*	@brief	Set material incident specular amount given an index-of-refraction
	*
	*	Sets specular term using a given index-of-refraction. Defaults to 1.5.
	*
	* 	@param ior	Index-of-refraction - range: [1,infinity) (Usually in range [1,2])
	*/
	void set_index_of_refraction(float ior) {
		descriptor.ior = ior;
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
		descriptor.sheen = s;
		Base::notify();
	}
	
	/**
	*	@brief	Set next layer
	*
	*	Layers are stacked top-to-bottom as a single linked list. This sets the next layer id (the layer directly under the current layer).
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
	float get_roughness() const { return descriptor.roughness; }
	float get_anisotropy() const { return anisotropy; }
	float get_metallic() const { return descriptor.metallic; }
	float get_index_of_refraction() const { return descriptor.ior; }
	float get_sheen() const { return descriptor.sheen; }

	auto *get_next_layer() const { return next_layer; }

	const material_layer_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
