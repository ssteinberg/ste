// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "material_descriptor.hpp"
#include "observable_resource.hpp"

#include "material_layer.hpp"

#include "Sampler.hpp"
#include "Texture2D.hpp"

#include "RGB.hpp"

#include <memory>
#include <functional>

namespace StE {
namespace Graphics {

/**
 *	@brief	Defines rendering material basic properties
 */
class material : public Core::observable_resource<material_descriptor> {
	using Base = Core::observable_resource<material_descriptor>;

private:
	Core::SamplerMipmapped material_sampler;

	std::shared_ptr<Core::Texture2D> cavity_map{ nullptr };
	std::shared_ptr<Core::Texture2D> normal_map{ nullptr };
	std::shared_ptr<Core::Texture2D> mask_map{ nullptr };

	material_layer *head_layer;

private:
	material_descriptor descriptor;

private:
	material(material &&m) = delete;
	material(const material &m) = delete;
	material &operator=(material &&m) = delete;
	material &operator=(const material &m) = delete;

private:
	Core::texture_handle handle_for_texture(const Core::Texture2D *t) const;

public:
	/**
	*	@brief	Material ctor
	*
	*	Constructs a new material using an upper layer. See set_layer method description for more information.
	*
	* 	@param head_layer	material layer
	*/
	explicit material(material_layer *head_layer);
	~material() {
		cavity_map = nullptr;
		normal_map = nullptr;
		mask_map = nullptr;
	}

	/**
	*	@brief	Set material cavity map
	*
	*	Cavity represents self-shadowing
	*
	* 	@param tex	2D texture object
	*/
	void set_cavity_map(const std::shared_ptr<Core::Texture2D> &tex) {
		cavity_map = tex;
		descriptor.cavity_handle = handle_for_texture(cavity_map.get());
		Base::notify();
	}

	/**
	*	@brief	Set material normal map
	*
	* 	@param tex	2D texture object
	*/
	void set_normal_map(const std::shared_ptr<Core::Texture2D> &tex) {
		normal_map = tex;
		descriptor.normal_handle = handle_for_texture(normal_map.get());
		Base::notify();
	}

	/**
	*	@brief	Set material mask map
	*
	*	Mask map is a binary alpha map. Texels with <.5 mask are discarded, otherwise mask values are ignored.
	*
	* 	@param tex	2D texture object
	*/
	void set_mask_map(const std::shared_ptr<Core::Texture2D> &tex) {
		mask_map = tex;
		descriptor.mask_handle = handle_for_texture(mask_map.get());
		Base::notify();
	}

	/**
	*	@brief	Set material emission RGB
	*
	*	Emission color of the material, defaults to { 0,0,0 }.
	*
	* 	@param rgb	Emission color
	*/
	void set_emission(const RGB &rgb) {
		descriptor.emission = rgb;
		Base::notify();
	}
	
	/**
	*	@brief	Set the head layer (top layer) of the layer stack
	*
	*	Materials are composed of variable thickness layers, see material_layer for more information about material layer parameters.
	*	Layers are stacked top to bottom (base) layer using a single linked list. This method set the id of the top layer, to link additional layers
	*	consult the set_next_layer method of material_layer. The layer which has no further layer linked is the base layer.
	*
	* 	@param layer	material layer
	*/
	void set_layer(material_layer *layer) {
		int layerid = material_layer_none;
		if (layer != nullptr) {
			auto id = layer->resource_index_in_storage();
			assert(id >= 0);
			if (id >= 0)
				layerid = id;
		}

		descriptor.layer_id = layerid;
		head_layer = layerid == material_layer_none ? nullptr : layer;
		
		Base::notify();
	}

	auto *get_cavity_map() const { return cavity_map.get(); }
	auto *get_normal_map() const { return normal_map.get(); }
	auto *get_mask_map() const { return mask_map.get(); }

	RGB get_emission() const { return descriptor.emission; }
	
	auto *get_head_layer() const { return head_layer; }

	const material_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
