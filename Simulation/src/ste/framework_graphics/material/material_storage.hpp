// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "Material.hpp"
#include "resource_storage_stable.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class material_storage : public Core::resource_storage_stable<material_descriptor> {
	using Base = resource_storage_stable<material_descriptor>;

public:
	std::unique_ptr<Material> allocate_material() {
		return Base::allocate_resource<Material>();
	}
	void erase_material(const Material *material) {
		erase_resource(material);
	}
};

}
}
