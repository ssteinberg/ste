// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <log.hpp>
#include <storage.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <texture.hpp>
#include <surface_factory.hpp>

#include <microfacet_refraction_fit.hpp>
#include <microfacet_transmission_fit.hpp>

namespace ste {
namespace graphics {

class material_lut_storage : public gl::storage<material_lut_storage> {
	static const char *refraction_fit_path;
	static const char *transmission_fit_path;

	static const char *ggx_tab_path;
	static const char *ggx_amp_path;

private:
	ste_resource<gl::texture<gl::image_type::image_2d>> microfacet_refraction_fit_lut;
	ste_resource<gl::texture<gl::image_type::image_2d_array>> microfacet_transmission_fit_lut;

	ste_resource<gl::texture<gl::image_type::image_2d>> ltc_ggx_fit;
	ste_resource<gl::texture<gl::image_type::image_2d>> ltc_ggx_amplitude;

private:
	template <typename T>
	struct load_lut {
		auto operator()(const char *name) const {
			using namespace text::attributes;

			std::ifstream ifs(name, std::ios::binary);
			if (!ifs.good()) {
				ste_log_error() << text::attributed_string("Can't open \"") + i(name) + "\": " + std::strerror(errno) << std::endl;
				throw std::runtime_error((lib::string(name) + " not found").c_str());
			}

			auto fit_data = T(ifs).create_lut();
			ifs.close();

			ste_log() << text::attributed_string("Loaded \"") + i(name) + "\" successfully." << std::endl;

			return fit_data;
		}
	};

public:
	material_lut_storage(const ste_context &ctx)
		: microfacet_refraction_fit_lut(ctx,
										resource::surface_factory::image_from_surface_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																														  load_lut<microfacet_refraction_fit>()(refraction_fit_path),
																														  gl::image_usage::sampled,
																														  gl::image_layout::shader_read_only_optimal,
																														  "Microfacet refraction fit LuT",
																														  false)),
		  microfacet_transmission_fit_lut(ctx,
										  resource::surface_factory::image_from_surface_2d_array<gl::format::r32g32b32a32_sfloat>(ctx,
																																  load_lut<microfacet_transmission_fit_v4>()(transmission_fit_path),
																																  gl::image_usage::sampled,
																																  gl::image_layout::shader_read_only_optimal,
																																  "Microfacet transmission fit LuT",
																																  false)),
		  ltc_ggx_fit(ctx,
					  resource::surface_factory::image_from_surface_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																										ggx_tab_path,
																										gl::image_usage::sampled,
																										gl::image_layout::shader_read_only_optimal,
																										"LTC GGX fit LuT",
																										false)),
		  ltc_ggx_amplitude(ctx,
							resource::surface_factory::image_from_surface_2d<gl::format::r32g32b32a32_sfloat>(ctx,
																											  ggx_amp_path,
																											  gl::image_usage::sampled,
																											  gl::image_layout::shader_read_only_optimal,
																											  "LTC GGX amplitude LuT",
																											  false)) {}

	/**
	 *	@brief	Returns a reference to the Microfacet refraction fit look-up-table.
	 *			Expects a nearest_clamp sampler.
	 */
	auto &get_microfacet_refraction_fit_lut() const { return microfacet_refraction_fit_lut.get(); }

	/**
	*	@brief	Returns a reference to the Microfacet transmission fit look-up-table.
	*			Expects a nearest_clamp sampler.
	*/
	auto &get_microfacet_transmission_fit_lut() const { return microfacet_transmission_fit_lut.get(); }

	/**
	*	@brief	Returns a reference to the LTC GGX fit look-up-table.
	*			Expects a linear_clamp sampler.
	*/
	auto &get_ltc_ggx_fit() const { return ltc_ggx_fit.get(); }

	/**
	*	@brief	Returns a reference to the LTC GGX amplitude look-up-table.
	*			Expects a linear_clamp sampler.
	*/
	auto &get_ltc_ggx_amplitude() const { return ltc_ggx_amplitude.get(); }
};

}
}
