// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <material.hpp>
#include <mesh.hpp>
#include <entity.hpp>

#include <signal.hpp>

#include <mesh_descriptor.hpp>

#include <lib/unique_ptr.hpp>

namespace ste {
namespace graphics {

class object_group;

class object : public entity_affine {
	using Base = entity_affine;

	friend class object_group;

public:
	using model_change_signal_type = signal<object*>;

private:
	mutable model_change_signal_type model_change_signal;
	mesh_descriptor md;

protected:
	const material *mat{ nullptr };
	lib::unique_ptr<mesh_generic> object_mesh;

public:
	object(lib::unique_ptr<mesh_generic> &&m) : object_mesh(std::move(m)) {}
	~object() noexcept {}

	mesh_generic &get_mesh() { return *object_mesh; }
	const mesh_generic &get_mesh() const { return *object_mesh; }

	void set_material(const material *m) {
		assert(m->is_valid() && "Orphaned Material");

		mat = m;
		model_change_signal.emit(this);
	}
	auto *get_material() const { return mat; }

public:
	auto &signal_model_change() const { return model_change_signal; }

	virtual void set_model_transform(const glm::mat4x3 &m) override {
		Base::set_model_transform(m);
		model_change_signal.emit(this);
	}
};

}
}
