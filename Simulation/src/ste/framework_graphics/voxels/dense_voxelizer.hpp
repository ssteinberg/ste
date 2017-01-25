// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "ste_engine_control.hpp"

#include "gpu_task.hpp"
#include "scene.hpp"

#include "glsl_program.hpp"
#include "glsl_program_factory.hpp"

namespace StE {
namespace Graphics {

class dense_voxel_space;

class dense_voxelizer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const dense_voxel_space *dvs;
	scene &s;

public:
	dense_voxelizer(const ste_engine_control &ctx, const dense_voxel_space *dvs, scene &s) : dvs(dvs), s(s) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
