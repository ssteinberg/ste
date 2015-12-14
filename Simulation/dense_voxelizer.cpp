
#include "stdafx.h"
#include "dense_voxelizer.h"
#include "dense_voxel_space.h"

#include "gl_current_context.h"

using namespace StE::Graphics;

void dense_voxelizer::prepare() const {
	LLR::gl_current_context::get()->color_mask(false, false, false, false);
	LLR::gl_current_context::get()->depth_mask(false);

	LLR::gl_current_context::get()->viewport(0, 0, dvs->size.x, dvs->size.x);
}

void dense_voxelizer::render() const {
	dvs->clear_space();
	scene(dvs->voxelizer_program, &dvs->voxelizer_fbo);

	LLR::gl_current_context::get()->memory_barrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	dvs->voxelizer_upsampler_program->bind();
	auto tiles_per_step = dvs->step_size / dvs->tile_size.x;
	auto center = dvs->size / 2u;

	for (std::size_t i = 0; i < dvs->mipmaps - 1; ++i, center /= 2u) {
		auto f = (tiles_per_step * dvs->tile_size).x;
		auto min = decltype(dvs->size)(glm::max(glm::ivec3(center - f), { 0,0,0 }));
		auto max = glm::min(center + f, center * 2u);
		auto count3 = max - min;
		unsigned count = count3.x >> 4;

		dvs->voxelizer_upsampler_program->set_uniform("tiles", int(f));
		dvs->voxelizer_upsampler_program->set_uniform("level", int(i));

		glDispatchCompute(count, count, count);

		LLR::gl_current_context::get()->memory_barrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void dense_voxelizer::finalize() const {
	LLR::gl_current_context::get()->color_mask(true, true, true, true);
	LLR::gl_current_context::get()->depth_mask(true);

	auto size = LLR::gl_current_context::get()->framebuffer_size();
	LLR::gl_current_context::get()->viewport(0, 0, size.x, size.y);
}
