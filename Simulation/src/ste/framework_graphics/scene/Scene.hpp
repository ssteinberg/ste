// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "gpu_task.hpp"

#include "SceneProperties.hpp"
#include "ObjectGroup.hpp"
#include "entity.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class Scene : public gpu_task {
	using Base = gpu_task;

private:
	ObjectGroup objects;
	SceneProperties scene_props;

public:
	Scene(Base::AccessToken, StEngineControl &ctx);
	~Scene() noexcept {}

	SceneProperties &scene_properties() { return scene_props; }
	const SceneProperties &scene_properties() const { return scene_props; }

	ObjectGroup &object_group() { return objects; }
	const ObjectGroup &object_group() const { return objects; }

public:
	static std::shared_ptr<Scene> create(StEngineControl &ctx) {
		return std::make_shared<Scene>(Base::AccessToken(), ctx);
	}
};

}
}
