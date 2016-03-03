// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "rendering_queue.h"

#include "GLSLProgram.h"

#include "Scene.h"

#include <memory>

namespace StE {
namespace Graphics {

class deferred_rendering_queue : public rendering_queue {
private:
	std::shared_ptr<GLSLProgram> scene_program;

public:
	deferred_rendering_queue() : scene_program(StE::Resource::GLSLProgramFactory::load_program_task(ctx, { "scene.vert", "scene.frag" })()) {}

	virtual void push_back(Scene* p) {
		p->set_program(scene_program);
	}

	virtual void push_back(std::unique_ptr<Scene> &&p) {
	}
};

}
}
