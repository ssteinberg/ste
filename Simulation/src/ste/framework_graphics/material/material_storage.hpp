// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "Material.hpp"
#include "resource_storage_stable.hpp"

#include <memory>
#include <vector>

namespace StE {
namespace Graphics {

class material_storage : public Core::resource_storage_stable<material_descriptor> {
	using Base = resource_storage_stable<material_descriptor>;

private:
	std::vector<std::unique_ptr<Material>> materials;

public:
	Material* allocate_material() {
		materials.push_back(Base::allocate_resource<Material>());
		return materials.back().get();
	}
	void erase_material(const Material *material) {
		erase_resource(material);
	}

	auto size() const { return materials.size(); }
	auto &get_materials() const { return materials; }
};

}
}
