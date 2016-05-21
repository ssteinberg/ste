// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "Texture2D.hpp"
#include "RGB.hpp"

#include <memory>

namespace StE {
namespace Graphics {

/**
 *	@brief	Defines rendering material
 */
class Material {
protected:
	std::shared_ptr<Core::Texture2D> basecolor_map;
	std::shared_ptr<Core::Texture2D> cavity_map;
	std::shared_ptr<Core::Texture2D> normal_map;
	std::shared_ptr<Core::Texture2D> mask_map;

	RGB emission{ .0f, .0f, .0f };
	float roughness{ .5f };
	float anisotropy{ .0f };
	float metallic{ .0f };
	float specular{ .5f };
	float sheen{ .0f };

public:
	/**
	*	@brief	Set material base color (diffuse) map
	*
	* 	@param tex	2D texture object
	*/
	void set_basecolor_map(const std::shared_ptr<Core::Texture2D> &tex) { basecolor_map = tex; }
	/**
	*	@brief	Set material cavity map
	*
	*	Cavity represents self-shadowing
	*
	* 	@param tex	2D texture object
	*/
	void set_cavity_map(const std::shared_ptr<Core::Texture2D> &tex) { cavity_map = tex; }
	/**
	*	@brief	Set material normal map
	*
	* 	@param tex	2D texture object
	*/
	void set_normal_map(const std::shared_ptr<Core::Texture2D> &tex) { normal_map = tex; }
	/**
	*	@brief	Set material mask map
	*
	*	Mask map is a binary alpha map. Texels with <.5 mask are discarded, otherwise mask values are ignored.
	*
	* 	@param tex	2D texture object
	*/
	void set_mask_map(const std::shared_ptr<Core::Texture2D> &tex) { mask_map = tex; }

	/**
	*	@brief	Set material emission RGB
	*
	*	Emission color of the material, defaults to { 0,0,0 }.
	*
	* 	@param tex	2D texture object
	*/
	void set_emission(const RGB &t) { emission = t; }
	/**
	*	@brief	Set material roughness
	*
	*	Roughness as defines by the Micorfacet theory. Controls both diffuse and specular response. Defaults to 0.5.
	*
	* 	@param tex	2D texture object
	*/
	void set_roughness(float t) { roughness = t; }
	/**
	*	@brief	Set material anisotropy
	*
	*	Anisotropy modifies material's anisotropic roughness
	*
	* 	@param tex	2D texture object
	*/
	void set_anisotropy(float t) { anisotropy = t; }
	/**
	*	@brief	Set material metallicity
	*
	*	Controls material's metal appearance. Defaults to 0.0.
	*
	* 	@param tex	2D texture object
	*/
	void set_metallic(float t) { metallic = t; }
	/**
	*	@brief	Set material incident specular amount
	*
	*	Controls material's index of refraction. Defaults to 0.5.
	*
	* 	@param tex	2D texture object
	*/
	void set_specular(float t) { specular = t; }
	/**
	*	@brief	Set material sheen
	*
	*	Sheen provides an additional cloth-like grazing component. Defaults to 0.0.
	*	Similiar to Disney's implementation.
	*
	* 	@param tex	2D texture object
	*/
	void set_sheen(float t) { sheen = t; }

	const Core::Texture2D *get_basecolor_map() const { return basecolor_map.get(); }
	const Core::Texture2D *get_cavity_map() const { return cavity_map.get(); }
	const Core::Texture2D *get_normal_map() const { return normal_map.get(); }
	const Core::Texture2D *get_mask_map() const { return mask_map.get(); }

	RGB get_emission() const { return emission; }
	float get_roughness() const { return roughness; }
	float get_anisotropy() const { return anisotropy; }
	float get_metallic() const { return metallic; }
	float get_specular() const { return specular; }
	float get_sheen() const { return sheen; }
};

}
}
