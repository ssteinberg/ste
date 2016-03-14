
#include "stdafx.h"
#include "dense_voxelizer.h"
#include "dense_voxel_space.h"

#include "gl_current_context.h"

using namespace StE::Graphics;

void dense_voxelizer::prepare() const {
	LLR::gl_current_context::get()->color_mask(false, false, false, false);
	LLR::gl_current_context::get()->depth_mask(false);

	LLR::gl_current_context::get()->viewport(0, 0, dvs->size.x * 2, dvs->size.x * 2);
}

void dense_voxelizer::render() const {
	dvs->clear_space();
	scene(dvs->voxelizer_program, &dvs->voxelizer_fbo);

	LLR::gl_current_context::get()->memory_barrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	
	dvs->voxelizer_upsampler_program->bind();
	auto center = dvs->size / 2;

	for (std::size_t i = 0; i < dvs->mipmaps - 1; ++i, center /= 2) {
		auto f = glm::min(center, dvs->step_size);
		auto min = center - f;
		auto max = center + f;
		auto count3 = max - min;
		auto count = count3 / 16;

		dvs->voxelizer_upsampler_program->set_uniform("tiles", int(f.x));
		dvs->voxelizer_upsampler_program->set_uniform("level", int(i));

		LLR::gl_current_context::get()->dispatch_compute(count.x, count.y, count.z);

		LLR::gl_current_context::get()->memory_barrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void dense_voxelizer::finalize() const {
	LLR::gl_current_context::get()->color_mask(true, true, true, true);
	LLR::gl_current_context::get()->depth_mask(true);

	auto size = LLR::gl_current_context::get()->framebuffer_size();
	LLR::gl_current_context::get()->viewport(0, 0, size.x, size.y);
}
