//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_exceptions.hpp>

#include <surface.hpp>
#include <opaque_surface.hpp>
#include <surface_type_traits.hpp>

#include <lib/vector.hpp>
#include <lib/shared_ptr.hpp>

namespace ste {
namespace resource {

class surface_convert {
private:
	struct _impl {
		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, std::size_t levels, std::size_t layers) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type()>)
				return Target(extent, layers, levels);
			else {
				assert(layers == 1);
				return Target(extent, levels);
			}
		}

		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, std::size_t levels, std::size_t layers, 
										  _detail::surface_storage<typename gl::format_traits<surface_format_v<Target>>::block_type> &&storage) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type()>)
				return Target(extent, layers, levels, storage);
			else {
				assert(layers == 1);
				return Target(extent, levels, storage);
			}
		}

		template <gl::format target_format, typename Target, typename CommonType, typename Src>
		static Target convert(Src &&surface, std::size_t(*src_block_loader)(const std::uint8_t *input, CommonType *output)) {
			if constexpr (std::is_same_v<Target, Src>) {
				// Identical surfaces, no need to convert
				assert(surface.surface_format() == target_format);
				return std::move(surface);
			}

			auto& src_traits = surface.get_format_traits();
			if (gl::format_traits<target_format>::is_compressed || src_traits.is_compressed) {
				// Format is a compressed format
				throw surface_convert_format_mismatch_error("Target or source format must not be a compressed type");
			}

			static_assert(gl::format_traits<target_format>::block_extent.x == 1 && gl::format_traits<target_format>::block_extent.y == 1);

			// Create target surface
			auto extent = surface.extent();
			auto levels = surface.levels();
			auto layers = surface.layers();
			auto elements = glm::max(src_traits.elements, gl::format_traits<target_format>::elements);
			Target target = create_target_surface<Target>(extent, layers, levels);

			lib::vector<CommonType> buffer;
			buffer.resize(src_traits.block_extent.x * src_traits.block_extent.y * src_traits.elements);

			// Convert blocks
			for (std::size_t a = 0; a < layers; ++a) {
				for (std::size_t l = 0; l < levels; ++l) {
					auto src_blocks_count = surface.blocks(l);
					auto dst_blocks_count = surface.blocks(l);

					auto src_layer_data = reinterpret_cast<const std::uint8_t*>(surface.data_at(a, l));
					auto dst_layer_data = target.data_at(a, l);

					auto src_extent_in_blocks = surface.extent_in_blocks(l);

					for (std::size_t b = 0; b < src_blocks_count; ++b) {
						// Decode each block into the temporary buffer
						auto written = src_block_loader(src_layer_data + b * src_traits.block_bytes, buffer.data());
						assert(written == buffer.size());

						// Write to target from temporary buffer
						for (std::uint32_t y = 0; y < src_traits.block_extent.y; ++y) {
							for (std::uint32_t x = 0; x < src_traits.block_extent.x; ++x) {
								glm::u32vec2 coord = { b % src_extent_in_blocks.x, b / src_extent_in_blocks.x };
								coord *= src_traits.block_extent;
								coord += glm::u32vec2{ x, y };

								auto p = dst_layer_data + coord.y * extent.x + coord.x;
								const auto decoded_block_src = buffer.data() + src_traits.elements * (x + y * src_traits.block_extent.x);
								p->template write_block<CommonType>(decoded_block_src, src_traits.elements);
							}
						}
					}
				}
			}

			return target;
		}

		template <gl::format target_format, typename Target, typename Src>
		static auto convert_opaque(Src &&surface) {
			static_assert(resource::is_opaque_surface_v<Src>);

			// Fast path, construct a surface directly from the opaque surface, if no conversion is required
			if (surface.surface_format() == target_format) {
				using block_type = typename gl::format_traits<target_format>::block_type;
				auto extent = surface.extent();
				auto levels = surface.levels();
				auto layers = surface.layers();

				// Take ownership of source's data buffer and create target surface storage
				const auto src_ptr = lib::shared_ptr<std::uint8_t>(std::move(surface.storage));
				auto src_ptr_block = lib::reinterpret_pointer_cast<block_type>(src_ptr);
				_detail::surface_storage<block_type> storage(std::move(src_ptr_block));

				// Create target surface with populated storage
				Target target = create_target_surface<Target>(extent, layers, levels, std::move(storage));
				return target;
			}

			// Query format's block loader function pointer and convert
			switch (surface.get_format_traits().block_common_type_name) {
			default:
				assert(false);
			case block_common_type::fp32: {
				auto loader = surface.get_format_traits().block_loader_fp32;
				return convert<target_format, Target>(std::move(surface), loader);
			}
			case block_common_type::fp64: {
				auto loader = surface.get_format_traits().block_loader_fp64;
				return convert<target_format, Target>(std::move(surface), loader);
			}
			case block_common_type::int32: {
				auto loader = surface.get_format_traits().block_loader_int32;
				return convert<target_format, Target>(std::move(surface), loader);
			}
			case block_common_type::int64: {
				auto loader = surface.get_format_traits().block_loader_int64;
				return convert<target_format, Target>(std::move(surface), loader);
			}
			case block_common_type::uint32: {
				auto loader = surface.get_format_traits().block_loader_uint32;
				return convert<target_format, Target>(std::move(surface), loader);
			}
			case block_common_type::uint64: {
				auto loader = surface.get_format_traits().block_loader_uint64;
				return convert<target_format, Target>(std::move(surface), loader);
			}
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
	static auto convert_1d(opaque_surface<1> &&surface) {
		return _impl::convert_opaque<target_format, surface_1d<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts 2D opaque surface to an 2D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_2d(opaque_surface<2> &&surface) {
		return _impl::convert_opaque<target_format, surface_2d<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts 3D opaque surface to an 3D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_3d(opaque_surface<3> &&surface) {
		return _impl::convert_opaque<target_format, surface_3d<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts 1D array opaque surface to an 1D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_1d_array(opaque_surface<1> &&surface) {
		return _impl::convert_opaque<target_format, surface_1d_array<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts 2D array opaque surface to an 2D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_2d_array(opaque_surface<2> &&surface) {
		return _impl::convert_opaque<target_format, surface_2d_array<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts cubemap opaque surface to a cubemap surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_cubemap(opaque_surface<2> &&surface) {
		return _impl::convert_opaque<target_format, surface_cubemap<target_format>>(std::move(surface));
	}

	/**
	*	@brief	Converts cubemap array opaque surface to a cubemap array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format>
	static auto convert_cubemap_array(opaque_surface<2> &&surface) {
		return _impl::convert_opaque<target_format, surface_cubemap_array<target_format>>(std::move(surface));
	}


	/**
	*	@brief	Converts a surface from source format to specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, typename Surface>
	static auto convert(Surface &&surface) {
		static constexpr auto target_image_type = resource::surface_image_type_v<Surface>;
		using Target = surface_for_image_type_t<target_image_type, target_format>;

		static_assert(resource::is_surface_v<Surface>);
		static_assert(resource::is_surface_v<Target>);

		static constexpr auto source_format = surface_format_v<Surface>;

		return _impl::convert<target_format, Target>(std::move(surface),
													 gl::format_traits<source_format>::block_type::load_block);
	}


	/**
	*	@brief	Converts 1D surface to an 1D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_1d(surface_1d<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts 2D surface to an 2D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_2d(surface_2d<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts 3D surface to an 3D surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_3d(surface_3d<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts 1D array surface to an 1D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_1d_array(surface_1d_array<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts 2D array surface to an 2D array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_2d_array(surface_2d_array<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts cubemap surface to a cubemap surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_cubemap(surface_cubemap<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}

	/**
	*	@brief	Converts cubemap array surface to a cubemap array surface of specified target format
	*
	*	@trows	surface_convert_format_mismatch_error	If can not convert input to target_format
	*/
	template <gl::format target_format, gl::format source_format>
	static auto convert_cubemap_array(surface_cubemap_array<source_format> &&surface) {
		return convert<target_format>(std::move(surface));
	}
};

}
}
