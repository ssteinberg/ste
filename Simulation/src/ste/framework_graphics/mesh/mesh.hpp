// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "object_vertex_data.hpp"
#include "vertex_buffer_object.hpp"
#include "vertex_array_object.hpp"
#include "element_buffer_object.hpp"

#include "mesh_aabb.hpp"
#include "mesh_bounding_sphere.hpp"

#include <vector>

#include <type_traits>
#include <functional>
#include <memory>

namespace StE {
namespace Graphics {

class mesh_generic {
public:
	virtual ~mesh_generic() noexcept {};

	virtual const std::vector<object_vertex_data> &get_vertices() const = 0;
	virtual const std::vector<std::uint32_t> &get_indices() const = 0;
	virtual const mesh_bounding_sphere &bounding_sphere() const = 0;
};

enum class mesh_subdivion_mode {
	Triangles = GL_TRIANGLES,
	TrianglesStrip = GL_TRIANGLE_STRIP,
	TrianglesFan = GL_TRIANGLE_FAN,
};

template<mesh_subdivion_mode Mode>
class mesh : public mesh_generic {
public:
	using vbo_type = StE::Core::vertex_buffer_object<StE::Graphics::object_vertex_data, StE::Graphics::object_vertex_data::descriptor>;
	using ebo_type = StE::Core::element_buffer_object<std::uint32_t>;

private:
	std::vector<object_vertex_data> vertices;
	std::vector<ebo_type::T> indices;

	std::unique_ptr<vbo_type> mesh_vbo;
	std::unique_ptr<ebo_type> mesh_ebo;
	std::unique_ptr<StE::Core::vertex_array_object> mesh_vao;

protected:
	mesh_bounding_sphere sphere;

private:
	void calc_sphere() {
		if (!vertices.size()) {
			sphere = mesh_bounding_sphere({ 0,0,0 }, .0f);
			return;
		}

		float d = 1.f / static_cast<float>(vertices.size());
		glm::vec3 c{ 0 };
		for (auto &v : vertices)
			c += v.p * d;

		float r = .0f;
		for (auto &v : vertices) {
			glm::vec3 u = v.p - c;
			r = glm::max(r, dot(u,u));
		}

		sphere = mesh_bounding_sphere(c, glm::sqrt(r));
	}

public:
	static constexpr mesh_subdivion_mode mode = Mode;

public:
	virtual ~mesh() noexcept {};

	const std::vector<object_vertex_data> &get_vertices() const override final { return vertices; }
	const std::vector<ebo_type::T> &get_indices() const override final { return indices; }
	const mesh_bounding_sphere &bounding_sphere() const override final { return sphere; };

	void set_vertices(const object_vertex_data *vert, int size) {
		vertices = decltype(vertices)(vert, vert + size);
		mesh_vbo = nullptr;
		calc_sphere();
	}
	void set_indices(const ebo_type::T *ind, int size) {
		indices = decltype(indices)(ind, ind + size);
		mesh_vbo = nullptr;
		calc_sphere();
	}
	template <typename T>
	void set_vertices(T &&vert) {
		vertices = decltype(vertices)(std::forward<T>(vert));
		mesh_vbo = nullptr;
		calc_sphere();
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
		calc_sphere();
		return *this;
	}

	mesh& operator*=(float scale) {
		for (auto &v : vertices) v.p *= scale;
		calc_sphere();
		return *this;
	}

	void transform(const glm::mat4 &m) {
		auto tim = glm::transpose(glm::inverse(m));
		for (auto &v : vertices) {
			auto tangent_frame = v.extract_tangent_frame();

			for (int i=0; i<3; ++i)
				tangent_frame[i] = glm::normalize((tim * glm::vec4(tangent_frame[i], 1)).xyz());

			v.p = (m * glm::vec4(v.p, 1)).xyz();
			v.tangent_frame_from_tbn(tangent_frame[0], tangent_frame[1], tangent_frame[2]);
		}
		calc_sphere();
	}

	const vbo_type *vbo() {
		if (mesh_vbo != nullptr)
			return mesh_vbo.get();

		return (mesh_vbo = std::make_unique<vbo_type>(get_vertices())).get();
	}

	const StE::Core::element_buffer_object<> *ebo() {
		if (mesh_ebo != nullptr)
			return mesh_ebo.get();

		return (mesh_ebo = std::make_unique<StE::Core::element_buffer_object<>>(get_indices())).get();
	}

	const StE::Core::vertex_array_object *vao() {
		if (mesh_vao != nullptr)
			return mesh_vao.get();

		vbo();

		mesh_vao = std::make_unique<StE::Core::vertex_array_object>();
		(*mesh_vao)[0] = (*mesh_vbo)[0];
		(*mesh_vao)[1] = (*mesh_vbo)[1];
		(*mesh_vao)[2] = (*mesh_vbo)[2];
		return mesh_vao.get();
	}
};

}
}
