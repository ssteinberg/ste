//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_exceptions.hpp>

#include <surface.hpp>
#include <opaque_surface.hpp>

namespace ste {
namespace resource {

class surface_convert {
private:
	struct _impl {
		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, std::size_t levels, std::size_t layers) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type>)
				return Target(extent, layers, levels);
			else {
				assert(layers == 1);
				return Target(extent, levels);
			}
		}

		template <gl::format target_format, typename Target, typename CommonType, typename Src>
		static auto convert(const Src &surface, std::size_t(*src_block_loader)(const std::uint8_t *input, CommonType *output)) {
			auto& src_traits = surface.get_format_traits();
			if (src_traits.elements != gl::format_traits<target_format>::elements) {
				// Wrong number of elements, can not convert
				throw surface_convert_format_mismatch_error("Source and target formats must have same element count");
			}
			if (gl::format_traits<target_format>::is_compressed || src_traits.is_compressed) {
				// Target format is a compressed format
				throw surface_convert_format_mismatch_error("Target or source format must not be a compressed type");
			}

			static_assert(gl::format_traits<target_format>::block_extent.x == 1 && gl::format_traits<target_format>::block_extent.y == 1);

			// Create target surface
			auto extent = surface.extent();
			auto levels = surface.levels();
			auto layers = surface.layers();
			Target target = create_target_surface<Target>(extent, layers, levels);

			const auto *src_data = reinterpret_cast<const std::uint8_t*>(surface.data());
			auto *dst_blocks = target.data();

			lib::vector<CommonType> buffer;
			buffer.resize(src_traits.block_extent.x * src_traits.block_extent.y * src_traits.elements);

			// Convert blocks
			for (std::size_t a = 0; a < layers; ++a) {
				for (std::size_t l = 0; l < levels; ++l) {
					auto src_offset_bytes = surface.offset_blocks(a, l) * surface.block_bytes();
					auto src_blocks_count = surface.blocks(l);
					auto dst_blocks_count = surface.blocks(l);

					auto src_layer_data = src_data + src_offset_bytes;
					auto dst_layer_data = target.data_at(a, l);

					auto src_extent_in_blocks = surface.extent_in_blocks(l);
					auto dst_extent_in_blocks = target.extent_in_blocks(l);

					for (std::size_t b = 0; b < src_blocks_count; ++b) {
						// Decode each block into the temporary buffer
						auto written = src_block_loader(src_layer_data + b * surface.block_bytes(), buffer.data());
						assert(written == buffer.size());

						// Write to target from temporary buffer
						for (unsigned y = 0; y < src_traits.block_extent.y; ++y) {
							for (unsigned x = 0; x < src_traits.block_extent.x; ++x) {
								const auto w = b % src_extent_in_blocks.x + x;
								const auto h = b / src_extent_in_blocks.x + y;

								auto p = dst_layer_data + h * dst_extent_in_blocks.x + w;
								p->template write_block<CommonType>(buffer.data());
							}
						}
					}
				}
			}

			return target;
		}

		template <gl::format target_format, template<gl::format> typename Target, typename Src>
		static auto convert_opaque(const Src &surface) {
			switch (surface.get_format_traits().block_common_type_name) {
			default:
				assert(false);
			case block_common_type::fp32:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_fp32);
			case block_common_type::fp64:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_fp64);
			case block_common_type::int32:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_int32);
			case block_common_type::int64:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_int64);
			case block_common_type::uint32:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_uint32);
			case block_common_type::uint64:
				return convert<target_format, Target<target_format>>(surface, surface.get_format_traits().block_loader_uint64);
			}
		}
	};

public:
	/**
	*	@brief	Converts 1D opaque surface to an 1D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_1d(const opaque_surface<1> &surface) {
		return _impl::convert_opaque<target_format, surface_1d>(surface);
	}

	/**
	*	@brief	Converts 2D opaque surface to an 2D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_2d(const opaque_surface<2> &surface) {
		return _impl::convert_opaque<target_format, surface_2d>(surface);
	}

	/**
	*	@brief	Converts 3D opaque surface to an 3D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_3d(const opaque_surface<3> &surface) {
		return _impl::convert_opaque<target_format, surface_3d>(surface);
	}

	/**
	*	@brief	Converts 1D array opaque surface to an 1D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_1d_array(const opaque_surface<1> &surface) {
		return _impl::convert_opaque<target_format, surface_1d_array>(surface);
	}

	/**
	*	@brief	Converts 2D array opaque surface to an 2D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_2d_array(const opaque_surface<2> &surface) {
		return _impl::convert_opaque<target_format, surface_2d_array>(surface);
	}

	/**
	*	@brief	Converts cubemap opaque surface to a cubemap surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_cubemap(const opaque_surface<2> &surface) {
		return _impl::convert_opaque<target_format, surface_cubemap>(surface);
	}

	/**
	*	@brief	Converts cubemap array opaque surface to a cubemap array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_cubemap_array(const opaque_surface<2> &surface) {
		return _impl::convert_opaque<target_format, surface_cubemap_array>(surface);
	}


	/**
	*	@brief	Converts a surface from source format to specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format, template<gl::format> typename Surface>
	static auto convert(const Surface<source_format> &surface) {
		return _impl::convert<target_format, Surface<target_format>>(surface, gl::format_traits<source_format>::block_type::load_block);
	}


	/**
	*	@brief	Converts 1D surface to an 1D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_1d(const surface_1d<source_format> &surface) {
		return convert<target_format, source_format, surface_1d>(surface);
	}

	/**
	*	@brief	Converts 2D surface to an 2D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_2d(const surface_2d<source_format> &surface) {
		return convert<target_format, source_format, surface_2d>(surface);
	}

	/**
	*	@brief	Converts 3D surface to an 3D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_3d(const surface_3d<source_format> &surface) {
		return convert<target_format, source_format, surface_3d>(surface);
	}

	/**
	*	@brief	Converts 1D array surface to an 1D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_1d_array(const surface_1d_array<source_format> &surface) {
		return convert<target_format, source_format, surface_1d_array>(surface);
	}

	/**
	*	@brief	Converts 2D array surface to an 2D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_2d_array(const surface_2d_array<source_format> &surface) {
		return convert<target_format, source_format, surface_2d_array>(surface);
	}

	/**
	*	@brief	Converts cubemap surface to a cubemap surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_cubemap(const surface_cubemap<source_format> &surface) {
		return convert<target_format, source_format, surface_cubemap>(surface);
	}

	/**
	*	@brief	Converts cubemap array surface to a cubemap array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_cubemap_array(const surface_cubemap_array<source_format> &surface) {
		return convert<target_format, source_format, surface_cubemap_array>(surface);
	}
};

}
}
