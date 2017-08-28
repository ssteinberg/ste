//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <material_storage.hpp>
#include <material_layer_storage.hpp>
#include <material_textures_storage.hpp>
#include <light_storage.hpp>

#include <command_recorder.hpp>

namespace ste {
namespace graphics {

class scene_properties {
private:
	material_storage materials;
	material_layer_storage material_layers;

	material_textures_storage material_textures;

	light_storage lights;

public:
	scene_properties(const ste_context &ctx) 
		: materials(ctx),
		material_layers(ctx,
						material_textures),
		lights(ctx)
	{}

	auto& materials_storage() { return materials; }
	auto& materials_storage() const { return materials; }
	auto& material_layers_storage() { return material_layers; }
	auto& material_layers_storage() const { return material_layers; }

	auto& material_textures_storage() { return material_textures; }
	auto& material_textures_storage() const { return material_textures; }

	auto& lights_storage() { return lights; }
	auto& lights_storage() const { return lights; }

	void update(gl::command_recorder &recorder) {
		materials.update(recorder);
		material_layers.update(recorder);
		lights.update(recorder);
	}
};

}
}
