//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <format.hpp>
#include <format_type_traits.hpp>

#include <image_type.hpp>
#include <image_type_traits.hpp>

#include <memory>

namespace ste {
namespace resource {

namespace _detail {

/**
 *	@brief	Surface shared storage
 */
template<typename block_type, bool is_const>
class surface_storage {
	friend class surface_storage<block_type, true>;

private:
	std::shared_ptr<block_type> data;
	std::size_t offset;

private:
	surface_storage(std::shared_ptr<block_type> data,
					std::size_t offset) : data(data), offset(offset) {}

public:
	surface_storage(std::size_t blocks)
		: data(new block_type[blocks],
			   std::default_delete<block_type[]>()),
		offset(0)
	{}

	template <bool b = is_const, typename = typename std::enable_if_t<b>>
	surface_storage(surface_storage<block_type, false> &&o) noexcept : data(std::move(o.data)), offset(o.offset) {}

	surface_storage(surface_storage&&) = default;
	surface_storage(const surface_storage&) = default;
	surface_storage &operator=(surface_storage&&) = default;
	surface_storage &operator=(const surface_storage&) = default;

	auto view(std::size_t offset) const { return surface_storage(data, offset); }

	template <bool b = is_const, typename = typename std::enable_if_t<!b>>
	block_type* get() { return data.get() + offset; }
	const block_type* get() const { return data.get() + offset; }
};

/**
*	@brief	Surface base class
*/
template<gl::format format, gl::image_type image_type>
class surface_base {
public:
	using traits = gl::format_traits<format>;
	static constexpr auto dimensions = gl::image_dimensions_v<image_type>;
	using extent_type = typename gl::image_extent_type<dimensions>::type;
	using block_type = typename traits::block_type;

private:
	extent_type surface_extent;
	std::size_t surface_levels;
	std::size_t surface_layers;

protected:
	surface_base(const extent_type &extent,
				 std::size_t levels,
				 std::size_t layers)
		: surface_extent(extent), surface_levels(levels), surface_layers(layers)
	{
		assert(layers > 0);
		assert(levels > 0);
	}

public:
	virtual ~surface_base() noexcept {}

	surface_base(surface_base&&) = default;
	surface_base(const surface_base&) = delete;
	surface_base &operator=(surface_base&&) = default;
	surface_base &operator=(const surface_base&) = delete;

	/**
	*	@brief	Returns a pointer to the surface data
	*/
	virtual const block_type* data() const = 0;

	/**
	*	@brief	Returns the extent size, in texels, of a level
	*
	*	@param	level	Surface level
	*/
	auto extent(std::size_t level = 0) const {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
						surface_extent >> static_cast<typename extent_type::value_type>(level));
	}
	/**
	*	@brief	Returns the extent size, in blocks, of a level
	*
	*	@param	level	Surface level
	*/
	auto extent_in_blocks(std::size_t level = 0) const {
		return glm::max(extent_type(static_cast<typename extent_type::value_type>(1)),
			(surface_extent / block_extent()) >> static_cast<typename extent_type::value_type>(level));
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
	*	@brief	Returns the extent size of a block
	*/
	auto block_extent() const {
		return gl::format_traits<format>::block_extent;
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
			level_offset += blocks(l);

		return blocks_layer() * layer + level_offset;
	}

	/**
	*	@brief	Returns the size, in bytes, of a block
	*/
	auto block_bytes() const {
		return sizeof(traits::element_type);
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
}
