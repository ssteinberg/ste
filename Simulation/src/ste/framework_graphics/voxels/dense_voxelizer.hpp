// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "StEngineControl.hpp"

#include "gpu_task.hpp"
#include "Scene.hpp"

#include "glsl_program.hpp"
#include "GLSLProgramFactory.hpp"

namespace StE {
namespace Graphics {

class dense_voxel_space;

class dense_voxelizer : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	const dense_voxel_space *dvs;
	Scene &scene;

public:
	dense_voxelizer(const StEngineControl &ctx, const dense_voxel_space *dvs, Scene &scene) : dvs(dvs), scene(scene) {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
