//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <image_type.hpp>
#include <surface_impl.hpp>

#include <cubemap_face.hpp>

namespace ste {
namespace resource {

namespace _detail {

/**
*	@brief	Cubemap surface
*/
template<gl::format format, bool is_const = false>
class surface_cubemap : public surface_base<format, gl::image_type::image_cubemap> {
	using Base = surface_base<format, gl::image_type::image_cubemap>;

	static constexpr auto image_type = gl::image_type::image_cubemap;

public:
	using surface_layer_type = surface<format, gl::image_type::image_2d>;

	using Base::extent_type;
	using Base::block_type;
	using Base::traits;

	static_assert(sizeof(block_type) == traits::block_bytes, "sizeof(block_type) != block_bytes");

	using layer_type = surface_image<format, extent_type, block_type*>;
	using const_layer_type = surface_image<format, extent_type, const block_type*>;

private:
	surface_storage<block_type, is_const> storage;

public:
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	surface_cubemap(const extent_type &extent,
					std::size_t levels = 1)
		: Base(extent, levels, 6),
		storage(Base::blocks_layer() * Base::layers())
	{}
	surface_cubemap(const extent_type &extent,
					std::size_t levels,
					const surface_storage<block_type, is_const> &storage)
		: Base(extent, levels, 6),
		storage(storage)
	{}
	~surface_cubemap() noexcept {}

	surface_cubemap(surface_cubemap&&) = default;
	surface_cubemap(const surface_cubemap&) = delete;
	surface_cubemap &operator=(surface_cubemap&&) = default;
	surface_cubemap &operator=(const surface_cubemap&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* data() { return storage.get(); }
	/**
	*	@brief	Returns a pointer to the surface data
	*/
	const block_type* data() const override final { return storage.get(); }

	/**
	*	@brief	Returns a pointer to the surface layer's level data
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* data_at(std::size_t layer, std::size_t level = 0) {
		return data() + Base::offset_blocks(layer, level);
	}
	using Base::data_at;

	/**
	*	@brief	Returns a cubemap face for the queried face index
	*
	*	@param	face	Cubemap face
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	auto operator[](cubemap_face face) {
		const auto face_index = static_cast<std::underlying_type_t<cubemap_face>>(face);
		return (*this)[face_index];
	}
	/**
	*	@brief	Returns a cubemap face for the queried face index
	*
	*	@param	face	Cubemap face
	*/
	auto operator[](cubemap_face face) const {
		const auto face_index = static_cast<std::underlying_type_t<cubemap_face>>(face);
		return (*this)[face_index];
	}

	/**
	*	@brief	Returns a cubemap face for the queried face index
	*
	*	@param	face_index	Cubemap face
	*/
	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	auto operator[](std::uint32_t face_index) {
		assert(face_index < Base::layers());
		return surface<format, image_type, false>(Base::extent(),
												  Base::levels(),
												  storage.view(Base::offset_blocks(face_index, 0)));
	}
	/**
	*	@brief	Returns a cubemap face for the queried face index
	*
	*	@param	face_index	Cubemap face
	*/
	auto operator[](std::uint32_t face_index) const {
		assert(face_index < Base::layers());
		return surface<format, image_type, true>(Base::extent(),
												 Base::levels(),
												 storage.view(Base::offset_blocks(face_index, 0)));
	}
};

}

}
}
