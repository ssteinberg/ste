// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "material.hpp"
#include "resource_storage_stable.hpp"

#include <memory>
#include <type_traits>

namespace StE {
namespace Graphics {

class material_storage : public Core::resource_storage_stable<material_descriptor> {
	using Base = resource_storage_stable<material_descriptor>;

public:
	template <typename ... Ts>
	std::unique_ptr<material> allocate_material(Ts&&... args) {
		return Base::allocate_resource<material>(std::forward<Ts>(args)...);
	}
	void erase_material(const material *mat) {
		erase_resource(mat);
	}
};

}
}
