//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <surface_exceptions.hpp>

#include <surface.hpp>
#include <opaque_surface.hpp>
#include <surface_type_traits.hpp>

#include <surface_block_load.hpp>
#include <surface_block_store.hpp>

#include <lib/vector.hpp>
#include <lib/shared_ptr.hpp>

namespace ste {
namespace resource {

class surface_convert {
private:
	struct _impl {
		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, levels_t levels, layers_t layers) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type()>)
				return Target(extent, layers, levels);
			else {
				assert(layers == 1_layer);
				return Target(extent, levels);
			}
		}

		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, levels_t levels, layers_t layers,
										  _detail::surface_storage<typename gl::format_traits<surface_format_v<Target>>::block_type> &&storage) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type()>)
				return Target(extent, layers, levels, storage);
			else {
				assert(layers == 1_layer);
				return Target(extent, levels, storage);
			}
		}

		template <gl::format target_format, typename Target, typename common_type, typename Src>
		static Target convert(Src &&surface,
							  block_load_8component_result_t<common_type>(*src_block_loader8_r)(const std::uint8_t*, unsigned),
							  block_load_8component_result_t<common_type>(*src_block_loader8_g)(const std::uint8_t*, unsigned),
							  block_load_8component_result_t<common_type>(*src_block_loader8_b)(const std::uint8_t*, unsigned),
							  block_load_8component_result_t<common_type>(*src_block_loader8_a)(const std::uint8_t*, unsigned)) {
			if constexpr (std::is_same_v<Target, Src>) {
				// Identical surfaces, no need to convert
				assert(surface.surface_format() == target_format);
				return std::move(surface);
			}

			auto &src_traits = surface.get_format_traits();
			if (gl::format_traits<target_format>::is_compressed || src_traits.is_compressed) {
				// Format is a compressed format
				throw surface_convert_format_mismatch_error("Target or source format must not be a compressed type");
			}
			if (src_traits.block_extent.x != 1 || src_traits.block_extent.y != 1) {
				// Block size isn't 1x1
				throw surface_convert_format_mismatch_error("Source block size must be 1x1");
			}

			static_assert(gl::format_traits<target_format>::block_extent.x == 1 && gl::format_traits<target_format>::block_extent.y == 1);

			// Create target surface
			auto extent = surface.extent();
			auto levels = surface.levels();
			auto layers = surface.layers();
			auto elements = glm::max(src_traits.elements, gl::format_traits<target_format>::elements);
			Target target = create_target_surface<Target>(extent, levels, layers);

			// Convert blocks
			for (auto a = 0_layers; a < layers; ++a) {
				for (auto l = 0_mips; l < levels; ++l) {
					auto src_blocks_count = surface.blocks(l);
					auto dst_blocks_count = surface.blocks(l);
					assert(src_blocks_count == dst_blocks_count);

					auto src_layer_data = reinterpret_cast<const std::uint8_t*>(surface.data_at(a, l));
					auto dst_layer_data = target.data_at(a, l);

					for (std::size_t b = 0; b < src_blocks_count; b += 8) {
						// Set source and destination
						auto src = src_layer_data + b * src_traits.block_bytes;
						auto dst = dst_layer_data + b;

						// Maximal count of blocks to decode (limited to 8)
						unsigned count = std::min(static_cast<unsigned>(src_blocks_count - b), 8u);

						// Load and store components
						if constexpr (gl::format_traits<target_format>::elements > 0) {
							// R
							if (src_block_loader8_r) {
								auto loader_result = src_block_loader8_r(src, count);
								resource::store_block_8component<gl::component_swizzle::r>(dst, loader_result.data, count);
							}
							else
								resource::store_block_8component_default<gl::component_swizzle::r>(dst, count);
						}
						if constexpr (gl::format_traits<target_format>::elements > 1) {
							// G
							if (src_block_loader8_g) {
								auto loader_result = src_block_loader8_g(src, count);
								resource::store_block_8component<gl::component_swizzle::g>(dst, loader_result.data, count);
							}
							else
								resource::store_block_8component_default<gl::component_swizzle::g>(dst, count);
						}
						if constexpr (gl::format_traits<target_format>::elements > 2) {
							// B
							if (src_block_loader8_b) {
								auto loader_result = src_block_loader8_b(src, count);
								resource::store_block_8component<gl::component_swizzle::b>(dst, loader_result.data, count);
							}
							else
								resource::store_block_8component_default<gl::component_swizzle::b>(dst, count);
						}
						if constexpr (gl::format_traits<target_format>::elements > 3) {
							// A
							if (src_block_loader8_a) {
								auto loader_result = src_block_loader8_a(src, count);
								resource::store_block_8component<gl::component_swizzle::a>(dst, loader_result.data, count);
							}
							else
								resource::store_block_8component_default<gl::component_swizzle::a>(dst, count);
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
				Target target = create_target_surface<Target>(extent, levels, layers, std::move(storage));
				return target;
			}

			// Query format's block loader function pointer and convert
			switch (surface.get_format_traits().block_common_type_name) {
			default:
				assert(false);
			case block_common_type::fp32: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_fp32<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp32<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp32<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp32<gl::component_swizzle::a>(surface.get_format_traits()));
			}
			case block_common_type::fp64: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_fp64<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp64<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp64<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_fp64<gl::component_swizzle::a>(surface.get_format_traits()));
			}
			case block_common_type::int32: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_i32<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i32<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i32<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i32<gl::component_swizzle::a>(surface.get_format_traits()));
			}
			case block_common_type::int64: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_i64<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i64<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i64<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_i64<gl::component_swizzle::a>(surface.get_format_traits()));
			}
			case block_common_type::uint32: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_u32<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u32<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u32<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u32<gl::component_swizzle::a>(surface.get_format_traits()));
			}
			case block_common_type::uint64: {
				return convert<target_format, Target>(std::move(surface),
													  gl::format_block_loader_8component_u64<gl::component_swizzle::r>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u64<gl::component_swizzle::g>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u64<gl::component_swizzle::b>(surface.get_format_traits()),
													  gl::format_block_loader_8component_u64<gl::component_swizzle::a>(surface.get_format_traits()));
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

		// Assign block's 8-component loader function pointers
		using Block = typename gl::format_traits<source_format>::block_type;
		if constexpr (Block::elements == 1)
			return _impl::convert<target_format, Target, Block::common_type>(std::move(surface),
																			 resource::load_block_8component_buffer<gl::component_swizzle::r, Block>,
																			 nullptr, nullptr, nullptr);
		if constexpr (Block::elements == 2)
			return _impl::convert<target_format, Target, Block::common_type>(std::move(surface),
																			 resource::load_block_8component_buffer<gl::component_swizzle::r, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::g, Block>,
																			 nullptr, nullptr);
		if constexpr (Block::elements == 3)
			return _impl::convert<target_format, Target, Block::common_type>(std::move(surface),
																			 resource::load_block_8component_buffer<gl::component_swizzle::r, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::g, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::b, Block>,
																			 nullptr);
		if constexpr (Block::elements == 4)
			return _impl::convert<target_format, Target, Block::common_type>(std::move(surface),
																			 resource::load_block_8component_buffer<gl::component_swizzle::r, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::g, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::b, Block>,
																			 resource::load_block_8component_buffer<gl::component_swizzle::a, Block>);
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
