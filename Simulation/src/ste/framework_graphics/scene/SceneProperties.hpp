// StE
// © Shlomi Steinberg, 2015

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
	material_storage& material_storage() { return materials; }
	light_storage& lights_storage() { return lights; }

	void pre_draw() {
		lights.update_storage();
	}
	void post_draw() {
		lights.lock_ranges();
	}
};

}
}
