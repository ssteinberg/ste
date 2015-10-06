// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "task.h"
#include "Scene.h"

#include <tinyobjloader/tiny_obj_loader.h>

namespace StE {
namespace Resource {

class ModelLoader {
private:
	~ModelLoader() {}

	static void process_model_mesh(optional<task_scheduler*> sched, const tinyobj::shape_t &shape, const std::vector<tinyobj::material_t> &materials, const std::string &dir, Graphics::Scene *scene);

public:
	static task<bool> load_model_task(const std::string &file_path, Graphics::Scene *scene);
};

}
}
