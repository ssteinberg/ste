// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "material_descriptor.hpp"
#include "observable_resource.hpp"

#include "Texture2D.hpp"
#include "Sampler.hpp"

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

	std::shared_ptr<Core::Texture2D> basecolor_map;
	std::shared_ptr<Core::Texture2D> cavity_map;
	std::shared_ptr<Core::Texture2D> normal_map;
	std::shared_ptr<Core::Texture2D> mask_map;

	float anisotropy{ .0f };
	float index_of_refraction{ 1.5f };

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
	/**
	*	@brief	Convert material anisotropy value to ratio which is used to adjust anisotropic roughness values
	*/
	static float convert_anisotropy_to_ratio(float ansio) {
		return ansio != .0f ? glm::sqrt(1.f - ansio * .9f) : 1.f;
	}

	/**
	*	@brief	Convert index-of-refraction to the specular reflection coefficient at normal incidence for
	*			Schlick's Fresnel approximation
	*/
	static float convert_ior_to_F0(float ior) {
		return glm::pow((1.f - ior) / (1.f + ior), 2.f);
	}

public:
	Material();

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
	*	Sets specular term using a given index-of-refraction. Assumes secondary media has IOR of 1. Defaults to 1.5.
	*
	* 	@param ior	Index-of-refraction - range: [1,infinity) (Usually in range [1,2])
	*/
	void set_index_of_refraction(float ior) {
		index_of_refraction = ior;
		descriptor.F0 = convert_ior_to_F0(ior);
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

	auto *get_basecolor_map() const { return basecolor_map.get(); }
	auto *get_cavity_map() const { return cavity_map.get(); }
	auto *get_normal_map() const { return normal_map.get(); }
	auto *get_mask_map() const { return mask_map.get(); }

	RGB get_emission() const { return descriptor.emission; }
	float get_roughness() const { return descriptor.roughness; }
	float get_anisotropy() const { return anisotropy; }
	float get_metallic() const { return descriptor.metallic; }
	float get_index_of_refraction() const { return index_of_refraction; }
	float get_sheen() const { return descriptor.sheen; }

	const material_descriptor &get_descriptor() const override final { return descriptor; }
};

}
}
