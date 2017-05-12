// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <ste_resource_traits.hpp>

#include <device_buffer.hpp>
#include <device_image.hpp>
#include <array.hpp>
#include <stable_vector.hpp>
#include <sampler.hpp>
#include <packaged_texture.hpp>
#include <framebuffer.hpp>
#include <std430.hpp>

//#include <hdr_bokeh_blur_task.hpp>
//#include <hdr_bloom_blury_task.hpp>
//#include <hdr_bloom_blurx_task.hpp>
//#include <hdr_tonemap_coc_task.hpp>
//#include <hdr_create_histogram_task.hpp>
//#include <hdr_compute_histogram_sums_task.hpp>
//#include <hdr_compute_minmax_task.hpp>

#include <signal.hpp>

#include <memory>

namespace ste {
namespace graphics {

class hdr_dof_postprocess : ste_resource_deferred_create_trait {
private:
	struct hdr_bokeh_parameters : gl::std430<std::int32_t, std::int32_t, float> {
		using Base = gl::std430<std::int32_t, std::int32_t, float>;
		using Base::Base;

		auto& lum_min() { return get<0>(); }
		auto& lum_max() { return get<1>(); }
		auto& focus() { return get<2>(); }
	};

	struct hdr_textures {
		gl::device_image<2> hdr_final_image;
		gl::texture<gl::image_type::image_2d> hdr_final_linear;
		gl::texture<gl::image_type::image_2d> hdr_final_linear_clamp;

		gl::packaged_texture<gl::image_type::image_1d> hdr_vision_properties_texture;
		gl::packaged_texture<gl::image_type::image_2d> hdr_image;
		gl::packaged_texture<gl::image_type::image_2d> hdr_bloom_image;
		gl::packaged_texture<gl::image_type::image_2d> hdr_bloom_blurx_image;
		gl::packaged_texture<gl::image_type::image_2d> hdr_lums;

		hdr_textures() = default;
	};

	static constexpr float vision_properties_max_lum = 10.f;
	static constexpr hdr_bokeh_parameters parameters_initial{ std::tuple<std::int32_t, std::int32_t, float>(0x7FFFFFFF, 0, 0) };

private:
	std::reference_wrapper<const ste_context> ctx;

	std::unique_ptr<hdr_textures> textures;

	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer;
	gl::array<hdr_bokeh_parameters> hdr_bokeh_param_buffer_prev;
	gl::array<std::uint32_t> histogram;
	gl::array<std::uint32_t> histogram_sums;

//	hdr_compute_minmax_task compute_minmax_task;
//	hdr_create_histogram_task create_histogram_task;
//	hdr_compute_histogram_sums_task compute_histogram_sums_task;
//	hdr_tonemap_coc_task tonemap_coc_task;
//	hdr_bloom_blurx_task bloom_blurx_task;
//	hdr_bloom_blury_task bloom_blury_task;
//	hdr_bokeh_blur_task bokeh_blur_task;

private:
	static gl::packaged_texture<gl::image_type::image_1d> create_hdr_vision_properties_texture(const ste_context &ctx);
	static std::unique_ptr<hdr_textures> create_hdr_textures(const ste_context &ctx);

public:
	hdr_dof_postprocess(const ste_context &ctx);
	~hdr_dof_postprocess() noexcept;

	auto &get_input_image() const { return hdr_final_image; }

	/**
	*	@brief	Set the camera aperture parameter. Those parameters affect the depth of field of the resulting image.
	*
	* 	@param diameter		Lens diameter in world units. Defaults to human eye pupil diameter which ranges from 2e-3 to 8e-3.
	*	@param focal_length	Focal length world units. Defaults to human eye focal length, about 23e-3.
	*/
	void set_aperture_parameters(float diameter, float focal_length) const {
		assert(diameter > .0f && "Lens diameter must be positive");
		assert(focal_length > .0f && "Focal length must be positive");

//		bokeh_blur.get().set_uniform("aperture_diameter", diameter);
//		bokeh_blur.get().set_uniform("aperture_focal_length", focal_length);
	}

	void resize(glm::ivec2 size);

	auto& get_exposure_params_buffer() const { return hdr_bokeh_param_buffer; }
	auto& get_histogram_buffer() const { return histogram; }
	auto& get_histogram_sums_buffer() const { return histogram_sums; }
};

}

//namespace resource {
//
//template <>
//class resource_loading_task<graphics::hdr_dof_postprocess> {
//	using R = graphics::hdr_dof_postprocess;
//
//public:
//	auto loader(const ste_engine_control &ctx, R* object) {
//			object->attach_handles();
//
//			object->hdr_tonemap_coc.get().set_uniform("hdr_vision_properties_texture", vision_handle);
//			object->hdr_bloom_blurx.get().set_uniform("dir", glm::vec2{ 1.f, .0f });
//			object->hdr_bloom_blury.get().set_uniform("dir", glm::vec2{ .0f, 1.f });
//			object->resize(ctx.get_backbuffer_size());
//	}
//};

}
