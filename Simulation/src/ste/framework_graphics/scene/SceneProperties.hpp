// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "material_storage.hpp"
#include "light_storage.hpp"

namespace StE {
namespace Graphics {

class SceneProperties {
private:
	material_storage materials;
	light_storage lights;

public:
	material_storage& materials_storage() { return materials; }
	const material_storage& materials_storage() const { return materials; }
	light_storage& lights_storage() { return lights; }
	const light_storage& lights_storage() const { return lights; }
};

}
}
