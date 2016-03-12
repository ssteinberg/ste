// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "StEngineControl.h"
#include "gpu_task.h"

#include "SceneProperties.h"
#include "ObjectGroup.h"
#include "entity.h"

#include "GLSLProgram.h"

#include <memory>
#include <vector>
#include <algorithm>

namespace StE {
namespace Graphics {

class Scene : public gpu_task, public entity_affine {
	using Base = gpu_task;
	
private:
	using ProjectionSignalConnectionType = StEngineControl::projection_change_signal_type::connection_type; 
	
private:
	SceneProperties scene_props;

	std::vector<std::shared_ptr<ObjectGroup>> objects; 
	std::shared_ptr<LLR::GLSLProgram> object_program;
	
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	Scene(const StEngineControl &ctx);
	
	virtual void add_object(const std::shared_ptr<ObjectGroup> &obj) { objects.emplace_back(obj); }
	virtual void remove_object(const std::shared_ptr<ObjectGroup> &obj) { objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end()); }

	gpu_state dispatch_state() const override final { return gpu_state(); }
	void update() const override final;
	void dispatch() const override final;

	SceneProperties &scene_properties() { return scene_props; }
	const SceneProperties &scene_properties() const { return scene_props; }
};

}
}
