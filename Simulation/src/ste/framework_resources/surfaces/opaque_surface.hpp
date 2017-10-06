//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_exceptions.hpp>

#include <format.hpp>
#include <format_rtti.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace resource {

template <std::uint32_t dimensions>
class opaque_surface {
	friend class surface_convert;

	struct ctor {};

public:
	using extent_type = typename gl::image_extent_type<dimensions>::type;

private:
	extent_type surface_extent;
	levels_t surface_levels;
	layers_t surface_layers;

	gl::format format;
	gl::image_type image_type;

	gl::format_rtti format_traits;

	lib::unique_ptr<std::uint8_t[]> storage;

private:
	opaque_surface(ctor,
				   const gl::format &format,
				   const gl::image_type &image_type,
				   const extent_type &extent,
				   levels_t levels,
				   layers_t layers)
		: surface_extent(extent),
		surface_levels(levels),
		surface_layers(layers),
		format(format),
		image_type(image_type),
		format_traits(gl::format_id(format))
	{
		// Sanity checks
		if (gl::image_dimensions_for_type(image_type) != dimensions)
			throw surface_opaque_storage_mismatch_error("Unexpected image_type");
		if (gl::image_is_cubemap_for_type(image_type) && static_cast<std::size_t>(layers % 6u) != 0)
			throw surface_opaque_storage_mismatch_error("Expected a cubemap or cubemap array but layers count doesn't match");
	}

public:
	opaque_surface(const gl::format &format,
				   const gl::image_type &image_type,
				   const extent_type &extent,
				   levels_t levels,
				   layers_t layers,
				   const std::uint8_t *data,
				   byte_t data_size)
		: opaque_surface(ctor(), format, image_type, extent, levels, layers)
	{
		if (bytes() != data_size)
			throw surface_opaque_storage_mismatch_error("Provided storage size does not match surface extent size, levels count, layers count and format");

		storage = lib::allocate_unique<std::uint8_t[]>(static_cast<std::size_t>(data_size));
		std::memcpy(storage.get(), data, static_cast<std::size_t>(data_size));
	}
	opaque_surface(const gl::format &format,
				   const gl::image_type &image_type,
				   const extent_type &extent,
				   levels_t levels,
				   layers_t layers,
				   lib::unique_ptr<std::uint8_t[]> &&data,
				   byte_t data_size)
		: opaque_surface(ctor(), format, image_type, extent, levels, layers)
	{
		if (bytes() != data_size)
			throw surface_opaque_storage_mismatch_error("Provided storage size does not match surface extent size, levels count, layers count and format");

		storage = std::move(data);
	}
	opaque_surface(const gl::format &format,
				   const gl::image_type &image_type,
				   const extent_type &extent,
				   levels_t levels,
				   layers_t layers)
		: opaque_surface(ctor(), format, image_type, extent, levels, layers)
	{
		storage = lib::allocate_unique<std::uint8_t[]>(bytes());
	}
	~opaque_surface() noexcept {}

	opaque_surface(opaque_surface&&) = default;
	opaque_surface &operator=(opaque_surface&&) = default;
	opaque_surface(const opaque_surface&) = default;
	opaque_surface &operator=(const opaque_surface&) = default;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	auto* data() { return storage.get(); }
	/**
	*	@brief	Returns a pointer to the surface data
	*/
	auto* data() const { return storage.get(); }

	/**
	*	@brief	Returns a pointer to the surface layer's level data
	*/
	auto* data_at(layers_t layer, levels_t level = 0_mips) {
		return data() + offset_blocks(layer, level);
	}
	/**
	*	@brief	Returns a pointer to the surface layer's level data
	*/
	const auto* data_at(layers_t layer, levels_t level = 0_mips) const {
		return data() + offset_blocks(layer, level);
	}

	/**
	*	@brief	Returns the extent size of a level
	*
	*	@param	level	Surface level
	*/
	auto extent(levels_t level = 0_mips) const {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						surface_extent >> static_cast<typename extent_type::value_type>(level));
	}
	/**
	*	@brief	Returns the extent size, in blocks, of a level
	*
	*	@param	level	Surface level
	*/
	auto extent_in_blocks(levels_t level = 0_mips) const {
		auto extent = surface_extent;
		extent.x /= block_extent().x;
		if constexpr (dimensions > 1) extent.y /= block_extent().y;

		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						extent >> static_cast<typename extent_type::value_type>(level));
	}
	/**
	*	@brief	Returns the levels count in the surface
	*/
	auto levels() const {
		return surface_levels;
	}
	/**
	*	@brief	Returns the layers count in the surface
	*/
	auto layers() const {
		return surface_layers;
	}
	/**
	*	@brief	Returns the surface image type
	*/
	auto surface_image_type() const { return image_type; }
	/**
	*	@brief	Returns the surface format
	*/
	auto surface_format() const { return format; }
	/**
	*	@brief	Returns the surface's dimensions count
	*/
	static constexpr auto surface_dimensions() { return dimensions; }

	/**
	*	@brief	Returns the extent size of a block
	*/
	auto block_extent() const {
		return format_traits.block_extent;
	}

	/**
	*	@brief	Returns the block count in a single surface level
	*/
	auto blocks(levels_t level) const {
		assert(level < levels());

		std::size_t b = 1;
		for (std::remove_cv_t<decltype(dimensions)> i = 0; i < dimensions; ++i)
			b *= extent(level)[i];

		return b;
	}

	/**
	*	@brief	Returns the count of blocks that compose the whole mipmap chain of a layer
	*/
	auto blocks_layer() const {
		std::size_t b = 0;
		for (auto l = 0_mip; l < levels(); ++l)
			b += blocks(l);

		return b;
	}

	/**
	*	@brief	Computes the offset, in blocks, to a specified level of a specified layer of the surface
	*/
	auto offset_blocks(layers_t layer, levels_t level) const {
		std::size_t level_offset = 0;
		for (auto l = 0_mip; l < level; ++l)
			level_offset += blocks(l);

		return blocks_layer() * static_cast<std::size_t>(layer) + level_offset;
	}

	/**
	*	@brief	Returns the size, in bytes, of a block
	*/
	auto block_bytes() const {
		return byte_t(format_traits.block_bytes);
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface data
	*/
	auto bytes() const {
		return block_bytes() * blocks_layer() * layers();
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface layer's level
	*/
	auto bytes(levels_t level) const {
		return block_bytes() * blocks(level);
	}

	/**
	 *	@brief	Returns the RTTI traits structure for the surface format
	 */
	auto& get_format_traits() const {
		return format_traits;
	}
};

}
}
