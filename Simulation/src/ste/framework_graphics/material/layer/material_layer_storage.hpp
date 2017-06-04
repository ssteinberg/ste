//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <image_vector.hpp>
#include <material_layer.hpp>
#include <resource_storage_stable.hpp>

#include <lib/unique_ptr.hpp>
#include <alias.hpp>

namespace ste {
namespace graphics {

class material_layer_storage : public gl::resource_storage_stable<material_layer_descriptor> {
	using Base = gl::resource_storage_stable<material_layer_descriptor>;

private:
	alias<const ste_context> ctx;
	alias<gl::image_vector<gl::image_type::image_2d>> material_texture_storage;

public:
	material_layer_storage(const ste_context &ctx,
						   gl::image_vector<gl::image_type::image_2d> &material_texture_storage) 
		: Base(ctx, gl::buffer_usage::storage_buffer),
		ctx(ctx),
		material_texture_storage(material_texture_storage)
	{}

	lib::unique_ptr<material_layer> allocate_layer() {
		return Base::allocate_resource<material_layer>();
	}
	void erase_layer(const material_layer *layer) {
		erase_resource(layer);
	}
};

}
}
