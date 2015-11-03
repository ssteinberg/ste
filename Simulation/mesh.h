// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "ObjectVertexData.h"

#include <vector>

#include <type_traits>
#include <functional>

namespace StE {
namespace Graphics {

enum class mesh_subdivion_mode {
	Triangles = GL_TRIANGLES,
	TrianglesStrip = GL_TRIANGLE_STRIP,
	TrianglesFan = GL_TRIANGLE_FAN,
};

template<mesh_subdivion_mode Mode>
class mesh {
protected:
	std::vector<ObjectVertexData> vertices;
	std::vector<unsigned> indices;

public:
	static constexpr mesh_subdivion_mode mode = Mode;

public:
	virtual ~mesh() noexcept {};

	const auto &get_vertices() const { return vertices; }
	const auto &get_indices() const { return indices; }

	std::enable_if_t<Mode == mesh_subdivion_mode::Triangles, mesh> &operator+=(const mesh<mode> &rhs) {
		vertices.insert(vertices.end(), rhs.vertices.begin(), rhs.vertices.end());
		auto offset = indices.size();
		indices.insert(indices.end(), rhs.indices.begin(), rhs.indices.end());
		for (int i = 0; i < offset; ++i) indices[i] += offset;
		return *this;
	}
};

}
}
