// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Material.h"
#include "mesh.h"

#include <memory>

namespace StE {
namespace Graphics {

class Object {
private:
	Material material;
	std::unique_ptr<mesh_generic> object_mesh;

	glm::mat4 model_mat{ 1.f };
	bool model_mat_dirty{ false };

public:
	Object(std::unique_ptr<mesh_generic> &&m) : object_mesh(std::move(m)) {}

	void set_material(Material &&m) { material = std::move(m); }
	const Material &get_material() const { return material; }
	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_model_transform(const glm::mat4 &m) {
		model_mat = m;
		model_mat_dirty = true;
	}
	const glm::mat4 &get_model_transform() const { return model_mat; }
	const glm::mat4 &get_model_transform_clear_dirty_flag() {
		model_mat_dirty = false;
		return model_mat;
	}
	bool is_model_dirty() const { return model_mat_dirty; }
};

}
}
