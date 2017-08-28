//	StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <image_vector.hpp>

namespace ste {
namespace graphics {

class material_texture {
	friend class material_textures_storage;

	using storage_t = gl::image_vector<gl::image_type::image_2d>;
	using texture_t = storage_t::texture_t;
	using slot_t = storage_t::value_type;

private:
	slot_t slot{ nullptr };

protected:
	material_texture(slot_t &&slot) : slot(std::move(slot)) {}

public:
	material_texture() = default;
	~material_texture() noexcept {}

	material_texture(material_texture&&) = default;
	material_texture(const material_texture&) = default;
	material_texture &operator=(material_texture&&) = default;
	material_texture &operator=(const material_texture&) = default;

	/**
	 *	@brief	Returns a reference to the texture
	 *			Calling on a unintialized material_texture object will result in null-pointer dereferencing
	 */
	const texture_t& texture() const { return slot->get(); }
	/**
	 *	@brief	Returns the index of the texture in the material textures storage
	 *			Calling on a unintialized material_texture object will result in null-pointer dereferencing
	 */
	auto texture_index() const { return slot->get_slot_idx(); }

	operator bool() const { return slot != nullptr; }
};

}
}
