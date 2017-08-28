//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <material_texture.hpp>
#include <image_vector.hpp>

namespace ste {
namespace graphics {

class material_textures_storage {
	using storage_t = gl::image_vector<gl::image_type::image_2d>;
	using texture_t = storage_t::texture_t;

private:
	storage_t storage;

public:
	/**
	 *	@brief	Allocates a new texture slot
	 */
	auto allocate_texture(texture_t &&texture) {
		auto slot = storage.allocate_slot(std::move(texture));
		return material_texture(std::move(slot));
	}

	/**
	 *	@brief	Returns a pipeline resource binder, which binds all the modified storage textures since last call.
	 */
	auto binder() const { return storage.binder(); }

	auto size() const { return storage.size(); }
};

}
}
