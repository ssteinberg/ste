// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"

#include "SceneProperties.hpp"
#include "ObjectGroup.hpp"
#include "entity.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene {//: public gpu_task, public entity_affine {
// 	using Base = gpu_task; 
	
private:
	SceneProperties scene_props;

public:	
// 	virtual void add_object(const std::shared_ptr<ObjectGroup> &obj) { add_dependency(obj); }
// 	virtual void remove_object(const std::shared_ptr<ObjectGroup> &obj) { remove_dependency(obj)); }

	SceneProperties &scene_properties() { return scene_props; }
	const SceneProperties &scene_properties() const { return scene_props; }

protected:
// 	void dispatch() const override final {};
};

}
}
