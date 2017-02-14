// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <material_layer.hpp>
#include <resource_storage_stable.hpp>

#include <memory>

namespace StE {
namespace Graphics {

class material_layer_storage : public Core::resource_storage_stable<material_layer_descriptor> {
	using Base = resource_storage_stable<material_layer_descriptor>;

public:
	std::unique_ptr<material_layer> allocate_layer() {
		return Base::allocate_resource<material_layer>();
	}
	void erase_layer(const material_layer *layer) {
		erase_resource(layer);
	}
};

}
}
