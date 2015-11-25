
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
	using namespace LLR;

	scene(dvs->voxelizer_program, &LLR::gl_current_context::get()->defaut_framebuffer());
}

void dense_voxelizer::finalize() const {
	LLR::gl_current_context::get()->color_mask(true, true, true, true);
	LLR::gl_current_context::get()->depth_mask(true);

	auto size = LLR::gl_current_context::get()->framebuffer_size();
	LLR::gl_current_context::get()->viewport(0, 0, size.x, size.y);
}
