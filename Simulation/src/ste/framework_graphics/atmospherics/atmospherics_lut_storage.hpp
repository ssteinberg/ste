// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <storage.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>
#include <atmospherics_precompute_scattering.hpp>

#include <texture.hpp>
#include <surface_factory.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class atmospherics_lut_storage : public gl::storage<atmospherics_lut_storage> {
	static const char *lut_path;

private:
	lib::unique_ptr<atmospherics_precompute_scattering> lut_loader;

	ste_resource<gl::texture<gl::image_type::image_2d_array>> atmospherics_optical_length_lut;
	ste_resource<gl::texture<gl::image_type::image_3d>> atmospherics_scatter_lut;
	ste_resource<gl::texture<gl::image_type::image_3d>> atmospherics_mie0_scatter_lut;
	ste_resource<gl::texture<gl::image_type::image_3d>> atmospherics_ambient_lut;

public:
	atmospherics_lut_storage(const ste_context &ctx)
		: lut_loader(lib::allocate_unique<atmospherics_precompute_scattering>(lut_path)),
		atmospherics_optical_length_lut(ctx,
										resource::surface_factory::image_from_surface_2d_array<gl::format::r32_sfloat>(ctx,
																													   lut_loader->create_optical_length_lut(),
																													   gl::image_usage::sampled,
																													   gl::image_layout::shader_read_only_optimal,
																													   false)),
		atmospherics_scatter_lut(ctx,
								 resource::surface_factory::image_from_surface_3d<gl::format::r32g32b32a32_sfloat>(ctx,
																												   lut_loader->create_scatter_lut(),
																												   gl::image_usage::sampled,
																												   gl::image_layout::shader_read_only_optimal,
																												   false)),
		atmospherics_mie0_scatter_lut(ctx,
									  resource::surface_factory::image_from_surface_3d<gl::format::r32g32b32a32_sfloat>(ctx,
																														lut_loader->create_mie0_scatter_lut(),
																														gl::image_usage::sampled,
																														gl::image_layout::shader_read_only_optimal,
																														false)),
		atmospherics_ambient_lut(ctx,
								 resource::surface_factory::image_from_surface_3d<gl::format::r32g32b32a32_sfloat>(ctx,
																												   lut_loader->create_ambient_lut(),
																												   gl::image_usage::sampled,
																												   gl::image_layout::shader_read_only_optimal,
																												   false))
	{
		lut_loader = nullptr;
	}

	// Use linear_clamp sampler
	auto& get_atmospherics_optical_length_lut() const { return atmospherics_optical_length_lut.get(); }
	// Use linear_clamp sampler
	auto& get_atmospherics_scatter_lut() const { return atmospherics_scatter_lut.get(); }
	// Use linear_clamp sampler
	auto& get_atmospherics_mie0_scatter_lut() const { return atmospherics_mie0_scatter_lut.get(); }
	// Use linear_clamp sampler
	auto& get_atmospherics_ambient_lut() const { return atmospherics_ambient_lut.get(); }
};

}
}
