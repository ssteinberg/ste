// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "Material.h"

namespace StE {
namespace Graphics {

class Object {
private:
	Material material;

	int vertices, indices;

public:
	Object(int vertices, int indices) : vertices(vertices), indices(indices) {}

	void set_material(Material &&m) { material = std::move(m); }
	const Material &get_material() const { return material; }
};

}
}
