// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "StEngineControl.h"

#include "renderable.h"
#include "Scene.h"

#include "GLSLProgram.h"
#include "GLSLProgramFactory.h"

namespace StE {
namespace Graphics {

class dense_voxel_space;

class dense_voxelizer {
private:
	const dense_voxel_space *dvs;
	Scene &scene;

public:
	dense_voxelizer(const StEngineControl &ctx, const dense_voxel_space *dvs, Scene &scene) : dvs(dvs), scene(scene) {}

	virtual void prepare() const override;
	virtual void render() const override;
	virtual void finalize() const override;
};

}
}
