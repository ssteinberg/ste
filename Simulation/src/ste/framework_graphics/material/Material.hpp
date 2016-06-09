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
 *	@brief	Defines rendering material
 */
class Material : public Core::observable_resource<material_descriptor> {
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
	Material(Material &&m) = delete;
	Material(const Material &m) = delete;
	Material &operator=(Material &&m) = delete;
	Material &operator=(const Material &m) = delete;

private:
	Core::texture_handle handle_for_texture(const Core::Texture2D *t) const;

public:
	Material();
	~Material() {
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
	*	@brief	Set material layer
	*
	*	Layers are stacked top-to-bottom as a single linked list. This sets the head layer id (top layer).
	*	To set lower layers, link the next layer to the head layer directly.
	*
	* 	@param layer	Material layer
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
