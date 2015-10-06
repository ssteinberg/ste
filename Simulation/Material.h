// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Texture2D.h"

#include <memory>

namespace StE {
namespace Graphics {

class Material {
protected:
	std::unique_ptr<LLR::Texture2D> diffuse;
	std::unique_ptr<LLR::Texture2D> specular;
	std::unique_ptr<LLR::Texture2D> heightmap;
	std::unique_ptr<LLR::Texture2D> alphamap;
	std::unique_ptr<LLR::Texture2D> tex0;
	std::unique_ptr<LLR::Texture2D> tex1;
	std::unique_ptr<LLR::Texture2D> tex2;
	std::unique_ptr<LLR::Texture2D> tex3;

public:
	template <typename ... Ts>
	void make_diffuse(Ts&&... args) { diffuse = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_specular(Ts&&... args) { specular = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_heightmap(Ts&&... args) { heightmap = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_alphamap(Ts&&... args) { alphamap = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_tex0(Ts&&... args) { tex0 = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_tex1(Ts&&... args) { tex1 = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_tex2(Ts&&... args) { tex2 = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }
	template <typename ... Ts>
	void make_tex3(Ts&&... args) { tex3 = std::make_unique<LLR::Texture2D>(std::forward<Ts>(args)...); }

	const LLR::Texture2D *get_diffuse() const { return diffuse.get(); }
	const LLR::Texture2D *get_specular() const { return specular.get(); }
	const LLR::Texture2D *get_heightmap() const { return heightmap.get(); }
	const LLR::Texture2D *get_alphamap() const { return alphamap.get(); }
	const LLR::Texture2D *get_tex0() const { return tex0.get(); }
	const LLR::Texture2D *get_tex1() const { return tex1.get(); }
	const LLR::Texture2D *get_tex2() const { return tex2.get(); }
	const LLR::Texture2D *get_tex3() const { return tex3.get(); }
};

}
}
