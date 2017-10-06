//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <surface.hpp>
#include <opaque_surface.hpp>
#include <surface_type_traits.hpp>

namespace ste {
namespace resource {

class surface_copy {
	struct _impl {
		template <typename Target>
		static auto create_target_surface(const typename Target::extent_type &extent, levels_t levels, layers_t layers) {
			if constexpr (gl::image_has_arrays_v<Target::surface_image_type()>)
				return Target(extent, layers, levels);
			else {
				assert(layers == 1_layers);
				return Target(extent, levels);
			}
		}
	};

public:
	/**
	*	@brief	Makes a copy of a surface
	*/
	template <typename Surface>
	static auto copy(const Surface &surface) {
		static_assert(resource::is_surface_v<Surface>);

		// Create new surface
		auto extent = surface.extent();
		auto levels = surface.levels();
		auto layers = surface.layers();
		Surface target = _impl::create_target_surface<Surface>(extent, levels, layers);

		auto bytes = surface.bytes();
		assert(target.bytes() == surface.bytes());

		// Copy data
		std::memcpy(target.data(), surface.data(), static_cast<std::size_t>(bytes));

		return target;
	}

	/**
	*	@brief	Makes a copy of a 1D surface
	*/
	template <gl::format format>
	static auto copy_1d(const surface_1d<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a 2D surface
	*/
	template <gl::format format>
	static auto copy_2d(const surface_2d<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a 3D surface
	*/
	template <gl::format format>
	static auto copy_3d(const surface_3d<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a 1D array
	*/
	template <gl::format format>
	static auto copy_1d_array(const surface_1d_array<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a 2D array
	*/
	template <gl::format format>
	static auto copy_2d_array(const surface_2d_array<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a cubemap surface
	*/
	template <gl::format format>
	static auto copy_cubemap(const surface_cubemap<format> &surface) {
		return copy(surface);
	}

	/**
	*	@brief	Makes a copy of a cubemap array
	*/
	template <gl::format format>
	static auto copy_cubemap_array(const surface_cubemap_array<format> &surface) {
		return copy(surface);
	}
};

}
}
