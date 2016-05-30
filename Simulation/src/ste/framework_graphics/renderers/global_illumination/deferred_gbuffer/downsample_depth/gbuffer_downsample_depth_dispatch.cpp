
#include "stdafx.hpp"
#include "gbuffer_downsample_depth_dispatch.hpp"

#include "Sampler.hpp"

using namespace StE::Graphics;

void gbuffer_downsample_depth_dispatch::set_context_state() const {
	using namespace Core;

	11_sampler_idx = *Sampler::SamplerNearestClamp();
	11_tex_unit = *gbuffer->get_depth_target();
	12_sampler_idx = *SamplerMipmapped::MipmappedSamplerNearestClamp();
	12_tex_unit = *gbuffer->get_downsampled_depth_target();

	program.get().bind();
}

void gbuffer_downsample_depth_dispatch::dispatch() const {
	using namespace Core;

	constexpr int jobs = 32;

	auto size = gbuffer->get_size() / 2;

	for (int i = 0; i < gbuffer->get_downsampled_depth_target()->get_levels(); ++i, size /= 2) {
		4_image_idx = gbuffer->get_downsampled_depth_target()->make_image(i).with_access(ImageAccessMode::Write);
		program.get().set_uniform("lod", i);

		auto s = (size + glm::ivec2(jobs - 1)) / jobs;

		if (i != 1) Core::GL::gl_current_context::get()->memory_barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		Core::GL::gl_current_context::get()->dispatch_compute(s.x, s.y, 1);
	}
}
