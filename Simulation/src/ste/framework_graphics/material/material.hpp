//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_descriptor.hpp>
#include <observable_resource.hpp>

#include <material_texture.hpp>
#include <material_layer.hpp>

#include <sampler.hpp>
#include <rgb.hpp>

namespace ste {
namespace graphics {

/**
 *	@brief	Defines rendering material basic properties
 */
class material : public gl::observable_resource<material_descriptor> {
	using Base = gl::observable_resource<material_descriptor>;
	using texture_storage = gl::image_vector<gl::image_type::image_2d>;

private:
	gl::sampler material_sampler;

	material_texture cavity_map;
	material_texture normal_map;
	material_texture mask_map;
	material_texture texture;

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
	void set_head_layer(material_layer *head_layer);

public:
	/**
	*	@brief	Material ctor
	*
	*	Constructs a new material using an upper layer. See attach_layer_stack method description for more information.
	*
	* 	@param head_layer	material head layer (top layer) of the layer stack
	*/
	explicit material(const ste_context &ctx,
					  material_layer *head_layer);
	~material() noexcept {}

	/**
	*	@brief	Set material texture
	*
	*	The material texture is used to add an additional texture detail on top of material layers' colors.
	*	The final color value after computing irradiance, reflection and subsurface scattering is multiplied by the texture sample.
	*	For correct results, primary material color should be expressed as layer color, with texture only modulating it.
	*
	* 	@param tex	2D texture object
	*/
	void set_texture(const material_texture &tex) {
		texture = tex;
		descriptor.texture_handle() = texture.texture_index();

		if (descriptor.texture_handle())	descriptor.material_flags() |= material_descriptor::material_has_texture_bit;
		else								descriptor.material_flags() &= ~material_descriptor::material_has_texture_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material cavity map
	*
	*	Cavity represents self-shadowing
	*
	* 	@param tex	2D texture object
	*/
	void set_cavity_map(const material_texture &tex) {
		cavity_map = tex;
		descriptor.cavity_handle() = cavity_map.texture_index();

		if (descriptor.cavity_handle())		descriptor.material_flags() |= material_descriptor::material_has_cavity_map_bit;
		else								descriptor.material_flags() &= ~material_descriptor::material_has_cavity_map_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material normal map
	*
	* 	@param tex	2D texture object
	*/
	void set_normal_map(const material_texture &tex) {
		normal_map = tex;
		descriptor.normal_handle() = normal_map.texture_index();

		if (descriptor.normal_handle())		descriptor.material_flags() |= material_descriptor::material_has_normal_map_bit;
		else								descriptor.material_flags() &= ~material_descriptor::material_has_normal_map_bit;

		Base::notify();
	}

	/**
	*	@brief	Set material mask map
	*
	*	Mask map is a binary alpha map. Texels with <.5 mask are discarded, otherwise mask values are ignored.
	*
	* 	@param tex	2D texture object
	*/
	void set_mask_map(const material_texture &tex) {
		mask_map = tex;
		descriptor.mask_handle() = mask_map.texture_index();

		if (descriptor.mask_handle())		descriptor.material_flags() |= material_descriptor::material_has_mask_map_bit;
		else								descriptor.material_flags() &= ~material_descriptor::material_has_mask_map_bit;

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

		const glm::vec3 v = rgb;
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

		if (enable)						descriptor.material_flags() |= material_descriptor::material_has_subsurface_scattering_bit;
		else							descriptor.material_flags() &= ~material_descriptor::material_has_subsurface_scattering_bit;

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

	auto& get_cavity_map() const { return cavity_map; }
	auto& get_normal_map() const { return normal_map; }
	auto& get_mask_map() const { return mask_map; }
	auto& get_texture() const { return texture; }

	rgb get_emission() const { return emission; }

	auto is_subsurface_scattering_enabled() const { return sss; }
	
	auto *get_head_layer() const { return head_layer; }

	material_descriptor get_descriptor() const override final { return descriptor; }
};

}
}
