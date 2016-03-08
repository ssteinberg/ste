// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "Material.h"
#include "mesh.h"
#include "entity.h"

#include <memory>

namespace StE {
namespace Graphics {

class Object : public entity_flagged {
	using Base = entity_flagged;
	
private:
	using Base::is_dirty;
	using Base::clear_dirty_flag;
	
private:
	bool material_dirty{ false };
	
protected:
	int material_id;
	std::unique_ptr<mesh_generic> object_mesh;

public:
	Object(std::unique_ptr<mesh_generic> &&m) : object_mesh(std::move(m)) {}

	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_material_id(int m) {
		material_id = m;
		material_dirty = true;
	}
	auto get_material_id() const { return material_id; }
	
	auto is_model_mat_dirty() const { return Base::is_dirty(); }
	void clear_model_mat_dirty_flag() { Base::clear_dirty_flag(); }
	auto is_material_id_dirty() const { return material_dirty; }
	void clear_material_id_dirty_flag() { material_dirty = false; }
};

}
}
