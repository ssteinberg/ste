// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "material_storage.hpp"
#include "material_layer_storage.hpp"
#include "light_storage.hpp"

namespace StE {
namespace Graphics {

class scene_properties {
private:
	material_storage materials;
	material_layer_storage material_layers;
	light_storage lights;

public:
	auto& materials_storage() { return materials; }
	auto& materials_storage() const { return materials; }
	auto& material_layers_storage() { return material_layers; }
	auto& material_layers_storage() const { return material_layers; }
	auto& lights_storage() { return lights; }
	auto& lights_storage() const { return lights; }

	void update() {
		materials.update();
		material_layers.update();
		lights.update();
	}
};

}
}
