// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>

#include <material_descriptor.hpp>
#include <observable_resource.hpp>

#include <material_layer.hpp>

#include <Sampler.hpp>
#include <texture_2d.hpp>

#include <rgb.hpp>

#include <memory>

namespace ste {
namespace graphics {

/**
 *	@brief	Defines rendering material basic properties
 */
class material : public Core::observable_resource<material_descriptor> {
	using Base = Core::observable_resource<material_descriptor>;

private:
	Core::sampler_mipmapped material_sampler;

	std::shared_ptr<Core::texture_2d> cavity_map{ nullptr };
	std::shared_ptr<Core::texture_2d> normal_map{ nullptr };
	std::shared_ptr<Core::texture_2d> mask_map{ nullptr };
	std::shared_ptr<Core::texture_2d> texture{ nullptr };

	rgb emission{ 0, 0, 0 };

	bool sss{ false };

	material_layer *head_layer;

private:
	material_descriptor descriptor;

private:
	material(material &&m) = delete;
	material(const material &m) = delete;
	material &operator=(material &&m) = delete;
	material &operator=(const material &m) = delete;

private:
	Core::texture_handle handle_for_texture(const Core::texture_2d *t) const;
	void set_head_layer(material_layer *head_layer);

public:
	/**
	*	@brief	Material ctor
	*
	*	Constructs a new material using an upper layer. See attach_layer_stack method description for more information.
	*
	* 	@param head_layer	material head layer (top layer) of the layer stack
	*/
	explicit material(material_layer *head_layer);
	~material() {
		cavity_map = nullptr;
		normal_map = nullptr;
		mask_map = nullptr;
		texture = nullptr;
	}

	/**
	*	@brief	Set material texture
	*
	*	The material texture is used to add an additional texture detail on top of material layers' colors.
	*	The final color value after computing irradiance, reflection and subsurface scattering is multiplied by the texture sample.
	*	For correct results, primary material color should be expressed as layer color, with texture only modulating it.
	*
	* 	@param tex	2D texture object
	*/
	void set_texture(const std::shared_ptr<Core::texture_2d> &tex) {
		texture = tex;
		descriptor.texture_handle = handle_for_texture(texture.get());

		if (descriptor.texture_handle)  descriptor.material_flags |= material_descriptor::material_has_texture_bit;
		else							descriptor.material_flags &= ~material_descriptor::material_has_texture_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material cavity map
	*
	*	Cavity represents self-shadowing
	*
	* 	@param tex	2D texture object
	*/
	void set_cavity_map(const std::shared_ptr<Core::texture_2d> &tex) {
		cavity_map = tex;
		descriptor.cavity_handle = handle_for_texture(cavity_map.get());

		if (descriptor.cavity_handle)   descriptor.material_flags |= material_descriptor::material_has_cavity_map_bit;
		else							descriptor.material_flags &= ~material_descriptor::material_has_cavity_map_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material normal map
	*
	* 	@param tex	2D texture object
	*/
	void set_normal_map(const std::shared_ptr<Core::texture_2d> &tex) {
		normal_map = tex;
		descriptor.normal_handle = handle_for_texture(normal_map.get());

		if (descriptor.normal_handle)	descriptor.material_flags |= material_descriptor::material_has_normal_map_bit;
		else							descriptor.material_flags &= ~material_descriptor::material_has_normal_map_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material mask map
	*
	*	Mask map is a binary alpha map. Texels with <.5 mask are discarded, otherwise mask values are ignored.
	*
	* 	@param tex	2D texture object
	*/
	void set_mask_map(const std::shared_ptr<Core::texture_2d> &tex) {
		mask_map = tex;
		descriptor.mask_handle = handle_for_texture(mask_map.get());

		if (descriptor.mask_handle)		descriptor.material_flags |= material_descriptor::material_has_mask_map_bit;
		else							descriptor.material_flags &= ~material_descriptor::material_has_mask_map_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material emission RGB
	*
	*	Emission color of the material, defaults to { 0,0,0 }.
	*
	* 	@param rgb	Emission color
	*/
	void set_emission(const rgb &rgb) {
		emission = rgb;

		glm::vec3 v = rgb;
		descriptor.set_emission(glm::vec4{ v.r, v.g, v.b, 1.f });
		Base::notify();
	}

	/**
	*	@brief	Enable/disable subsurface-scattering.
	*
	*	Due to the increased cost of the subsurface scattering path (even when all layers' attenuation is +infinity),
	*	subsurface-scattering has to be explicitly enabled.
	*
	* 	@param enable	Toggles subsurface scattering
	*/
	void enable_subsurface_scattering(bool enable) {
		sss = enable;

		if (enable)						descriptor.material_flags |= material_descriptor::material_has_subsurface_scattering_bit;
		else							descriptor.material_flags &= ~material_descriptor::material_has_subsurface_scattering_bit;

		Base::notify();
	}
	
	/**
	*	@brief	Set the material layer stack
	*
	*	Materials are composed of variable thickness layers, see material_layer for more information about material layer parameters.
	*	Layers are stacked top to bottom (base) layer using a single linked list. This method set the id of the top layer, to link additional layers
	*	consult the set_next_layer method of material_layer. The layer which has no further linked layer is the base layer.
	*
	* 	@param head_layer	material head layer (top layer) of the layer stack
	*/
	void attach_layer_stack(material_layer *head_layer);

	auto *get_cavity_map() const { return cavity_map.get(); }
	auto *get_normal_map() const { return normal_map.get(); }
	auto *get_mask_map() const { return mask_map.get(); }
	auto *get_texture() const { return texture.get(); }

	rgb get_emission() const { return emission; }

	auto is_subsurface_scattering_enabled() const { return sss; }
	
	auto *get_head_layer() const { return head_layer; }

	const material_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
