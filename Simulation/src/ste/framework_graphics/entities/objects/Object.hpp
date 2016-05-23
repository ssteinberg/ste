// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "Material.hpp"
#include "mesh.hpp"
#include "entity.hpp"

#include "signal.hpp"

#include "mesh_descriptor.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class ObjectGroup;

class Object : public entity_affine {
	using Base = entity_affine;

	friend class ObjectGroup;

public:
	using signal_type = signal<Object*>;

private:
	signal_type model_change_signal;
	mesh_descriptor md;

protected:
	int material_id;
	std::unique_ptr<mesh_generic> object_mesh;

public:
	Object(std::unique_ptr<mesh_generic> &&m) : object_mesh(std::move(m)) {}
	~Object() noexcept {}

	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_material_id(int m) { material_id = m; }
	auto get_material_id() const { return material_id; }

public:
	const signal_type &signal_model_change() const { return model_change_signal; }

	virtual void set_model_transform(const glm::mat4x3 &m) override {
		Base::set_model_transform(m);
		model_change_signal.emit(this);
	}
};

}
}
