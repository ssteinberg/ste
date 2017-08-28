//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <material.hpp>
#include <resource_storage_stable.hpp>

#include <lib/unique_ptr.hpp>
#include <type_traits>

namespace ste {
namespace graphics {

class material_storage : public gl::resource_storage_stable<material_descriptor> {
	using Base = gl::resource_storage_stable<material_descriptor>;

public:
	material_storage(const ste_context &ctx) : Base(ctx, gl::buffer_usage::storage_buffer) {}

	template <typename ... Ts>
	lib::unique_ptr<material> allocate_material(Ts&&... args) {
		return Base::allocate_resource<material>(std::forward<Ts>(args)...);
	}
	void erase_material(const material *mat) {
		erase_resource(mat);
	}
};

}
}
