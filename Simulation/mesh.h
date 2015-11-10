// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include "ObjectVertexData.h"
#include "VertexBufferObject.h"
#include "VertexArrayObject.h"
#include "ElementBufferObject.h"

#include "mesh_aabb.h"

#include <vector>

#include <type_traits>
#include <functional>
#include <memory>

namespace StE {
namespace Graphics {

class mesh_generic {
public:
	virtual ~mesh_generic() noexcept {};

	virtual const std::vector<ObjectVertexData> &get_vertices() const = 0;
	virtual const std::vector<unsigned> &get_indices() const = 0;
	virtual const mesh_aabb &aabb() const = 0;
};

enum class mesh_subdivion_mode {
	Triangles = GL_TRIANGLES,
	TrianglesStrip = GL_TRIANGLE_STRIP,
	TrianglesFan = GL_TRIANGLE_FAN,
};

template<mesh_subdivion_mode Mode>
class mesh : public mesh_generic {
public:
	using vbo_type = StE::LLR::VertexBufferObject<StE::Graphics::ObjectVertexData, StE::Graphics::ObjectVertexData::descriptor>;
	using ebo_type = StE::LLR::ElementBufferObject<unsigned>;

private:
	std::vector<ObjectVertexData> vertices;
	std::vector<ebo_type::T> indices;

	std::unique_ptr<vbo_type> mesh_vbo;
	std::unique_ptr<ebo_type> mesh_ebo;
	std::unique_ptr<StE::LLR::VertexArrayObject> mesh_vao;

protected:
	mesh_aabb m_aabb;

private:
	void calc_aabb() {
		if (!vertices.size()) {
			m_aabb = mesh_aabb({ 0,0,0 }, { 0,0,0 });
			return;
		}

		glm::vec3 a = vertices.begin()->p, b = vertices.begin()->p;
		for (auto &v : vertices) {
			a = glm::min(a, v.p);
			b = glm::max(b, v.p);
		}
		m_aabb = mesh_aabb(a, b);
	}

public:
	static constexpr mesh_subdivion_mode mode = Mode;

public:
	virtual ~mesh() noexcept {};

	const std::vector<ObjectVertexData> &get_vertices() const override { return vertices; }
	const std::vector<ebo_type::T> &get_indices() const override { return indices; }
	const mesh_aabb &aabb() const override { return m_aabb; }

	template <typename T>
	void set_vertices(T &&vert) {
		vertices = decltype(vertices)(std::forward<T>(vert));
		mesh_vbo = nullptr;
		calc_aabb();
	}
	template <typename T>
	void set_indices(T &&ind) {
		indices = decltype(indices)(std::forward<T>(ind));
		mesh_ebo = nullptr;
	}

	template <bool b = Mode == mesh_subdivion_mode::Triangles>
	std::enable_if_t<b, mesh> &operator+=(const mesh<mode> &rhs) {
		vertices.insert(vertices.end(), rhs.vertices.begin(), rhs.vertices.end());
		auto offset = indices.size();
		indices.insert(indices.end(), rhs.indices.begin(), rhs.indices.end());
		for (int i = 0; i < offset; ++i) indices[i] += offset;
		calc_aabb();
		return *this;
	}

	void transform(const glm::mat4 &m) {
		auto tim = glm::transpose(glm::inverse(m));
		for (auto &v : vertices) {
			v.p = (m * glm::vec4(v.p, 1)).xyz;
			v.n = glm::normalize((tim * glm::vec4(v.n, 1)).xyz);
			v.t = glm::normalize((m * glm::vec4(v.t, 1)).xyz);
		}
	}

	const vbo_type *vbo() {
		if (mesh_vbo != nullptr)
			return mesh_vbo.get();

		return (mesh_vbo = std::make_unique<vbo_type>(get_vertices())).get();
	}

	const StE::LLR::ElementBufferObject<> *ebo() {
		if (mesh_ebo != nullptr)
			return mesh_ebo.get();

		return (mesh_ebo = std::make_unique<StE::LLR::ElementBufferObject<>>(get_indices())).get();
	}

	const StE::LLR::VertexArrayObject *vao() {
		if (mesh_vao != nullptr)
			return mesh_vao.get();

		vbo();

		mesh_vao = std::make_unique<StE::LLR::VertexArrayObject>();
		(*mesh_vao)[0] = (*mesh_vbo)[0];
		(*mesh_vao)[1] = (*mesh_vbo)[1];
		(*mesh_vao)[2] = (*mesh_vbo)[2];
		(*mesh_vao)[3] = (*mesh_vbo)[3];
		return mesh_vao.get();
	}
};

}
}
