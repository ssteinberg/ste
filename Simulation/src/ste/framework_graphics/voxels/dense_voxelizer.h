// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "gpu_task.h"
#include "Scene.h"

#include "GLSLProgram.h"
#include "GLSLProgramFactory.h"

namespace StE {
namespace Graphics {

class dense_voxel_space;

class dense_voxelizer : public gpu_task {
	using Base = gpu_task;
	
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
