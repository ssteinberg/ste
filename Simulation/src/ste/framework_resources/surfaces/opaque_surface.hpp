//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_exceptions.hpp>

#include <format.hpp>
#include <format_rtti.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace resource {

template <std::uint32_t dimensions>
class opaque_surface {
public:
	using extent_type = typename gl::image_extent_type<dimensions>::type;

private:
	extent_type surface_extent;
	std::size_t surface_levels;
	std::size_t surface_layers;

	gl::format surface_format;
	gl::image_type surface_image_type;

	gl::format_rtti format_traits;

	lib::vector<std::uint8_t> storage;

public:
	opaque_surface(const gl::format &format,
				   const gl::image_type &image_type,
				   const extent_type &extent,
				   std::size_t levels,
				   std::size_t layers,
				   lib::vector<std::uint8_t> &&data)
		: surface_extent(extent), 
		surface_levels(levels),
		surface_layers(layers),
		surface_format(format),
		surface_image_type(image_type),
		format_traits(gl::format_id(format)),
		storage(std::move(data)) 
	{
		// Sanity checks
		if (gl::image_dimensions_for_type(image_type) != dimensions)
			throw surface_opaque_storage_mismatch_error("Unexpected image_type");
		if (bytes() != data.size())
			throw surface_opaque_storage_mismatch_error("Provided storage size does not match surface extent size, levels count, layers count and format");
		if (gl::image_is_cubemap_for_type(image_type) && (layers % 6) != 0)
			throw surface_opaque_storage_mismatch_error("Expected a cubemap or cubemap array but layers count doesn't match");
	}
	~opaque_surface() noexcept {}

	opaque_surface(opaque_surface&&) = default;
	opaque_surface &operator=(opaque_surface&&) = default;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	auto* data() { return storage.data(); }
	/**
	*	@brief	Returns a pointer to the surface data
	*/
	auto* data() const { return storage.data(); }

	/**
	*	@brief	Returns the extent size of a level
	*
	*	@param	level	Surface level
	*/
	auto extent(std::size_t level = 0) const {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						surface_extent >> static_cast<typename extent_type::value_type>(level));
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
	auto image_type() const { return image_type; }
	/**
	*	@brief	Returns the surface format
	*/
	auto format() const { return format; }

	/**
	*	@brief	Returns the extent size of a block
	*/
	auto block_extent() const {
		return extent_type(static_cast<typename extent_type::value_type>(1));
	}

	/**
	*	@brief	Returns the block count in a single surface level
	*/
	auto blocks(std::size_t level) const {
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
		for (std::size_t l = 0; l < levels(); ++l)
			b += blocks(l);

		return b;
	}

	/**
	*	@brief	Computes the offset, in blocks, to a specified level of a specified layer of the surface
	*/
	auto offset_blocks(std::size_t layer, std::size_t level) const {
		std::size_t level_offset = 0;
		for (std::size_t l = 0; l < level; ++l)
			level_offset += blocks(l) * block_bytes();

		return blocks_layer() * layer + level_offset;
	}

	/**
	*	@brief	Returns the size, in bytes, of a block
	*/
	auto block_bytes() const {
		return format_traits.texel_bytes;
	}

	/**
	*	@brief	Returns the size, in bytes, of the surface data
	*/
	auto bytes() const {
		return block_bytes() * blocks_layer() * layers();
	}
};

}
}
