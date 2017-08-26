// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <storage.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <hdr_dof_bokeh_parameters.hpp>

#include <texture.hpp>
#include <array.hpp>
#include <std430.hpp>

#include <human_vision_properties.hpp>
#include <surface_factory.hpp>

namespace ste {
namespace graphics {

class hdr_dof_postprocess_storage : public gl::storage<hdr_dof_postprocess_storage> {
	friend class hdr_dof_postprocess;

private:
	static hdr_bokeh_parameters parameters_initial;
	static constexpr float vision_properties_max_lum = 10.f;

	ste_resource<gl::texture<gl::image_type::image_2d>> create_hdr_vision_properties_texture(const ste_context &ctx) {
		static constexpr auto format = gl::format::r32g32b32a32_sfloat;
		static constexpr auto gli_format = gl::format_traits<format>::gli_format;

		gli::texture2d hdr_human_vision_properties_data(gli_format, glm::tvec2<std::size_t>{ 4096, 1 }, 1);
		{
			glm::vec4 *d = reinterpret_cast<glm::vec4*>(hdr_human_vision_properties_data.data());
			for (int i = 0; i < hdr_human_vision_properties_data.extent().x; ++i, ++d) {
				const float x = (static_cast<float>(i) + .5f) / static_cast<float>(hdr_human_vision_properties_data.extent().x);
				const float l = glm::mix(ste::graphics::human_vision_properties::min_luminance,
										 vision_properties_max_lum,
										 x);
				*d = {
					ste::graphics::human_vision_properties::scotopic_vision(l),
					ste::graphics::human_vision_properties::mesopic_vision(l),
					ste::graphics::human_vision_properties::monochromaticity(l),
					ste::graphics::human_vision_properties::visual_acuity(l)
				};
			}
		}

		auto image = resource::surface_factory::image_from_surface_2d<format>(ctx,
																			  std::move(hdr_human_vision_properties_data),
																			  gl::image_usage::sampled,
																			  gl::image_layout::shader_read_only_optimal,
																			  false);
		return ste_resource<gl::texture<gl::image_type::image_2d>>(ctx, std::move(image));
	}

public:
	ste_resource<gl::texture<gl::image_type::image_2d>> hdr_vision_properties_texture;

	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer;
	//	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev;
	gl::array<gl::std430<std::uint32_t>> histogram;
	gl::array<gl::std430<std::uint32_t>> histogram_sums;

public:
	hdr_dof_postprocess_storage(const ste_context &ctx) 
		: hdr_vision_properties_texture(create_hdr_vision_properties_texture(ctx)),
		// Buffers
		hdr_bokeh_param_buffer(ctx, 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
//		hdr_bokeh_param_buffer_prev(ctx, 1, { parameters_initial }, gl::buffer_usage::storage_buffer),
		histogram(ctx, 128, gl::buffer_usage::storage_buffer),
		histogram_sums(ctx, 128, gl::buffer_usage::storage_buffer)
	{}
	~hdr_dof_postprocess_storage() noexcept {}
};

}
}
