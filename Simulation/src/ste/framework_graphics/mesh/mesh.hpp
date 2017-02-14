// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <object_vertex_data.hpp>
#include <mesh_bounding_sphere.hpp>

#include <vector>
#include <type_traits>

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
	Triangles,
	TrianglesStrip,
	TrianglesFan,
};

template<mesh_subdivion_mode Mode>
class mesh : public mesh_generic {
private:
	std::vector<object_vertex_data> vertices;
	std::vector<std::uint32_t> indices;

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
	const std::vector<std::uint32_t> &get_indices() const override final { return indices; }
	const mesh_bounding_sphere &bounding_sphere() const override final { return sphere; };

	void set_vertices(const object_vertex_data *vert, int size) {
		vertices = decltype(vertices)(vert, vert + size);
		calc_sphere();
	}
	void set_indices(const std::uint32_t *ind, int size) {
		indices = decltype(indices)(ind, ind + size);
		calc_sphere();
	}
	template <typename T>
	void set_vertices(T &&vert) {
		vertices = decltype(vertices)(std::forward<T>(vert));
		calc_sphere();
	}
	template <typename T>
	void set_indices(T &&ind) {
		indices = decltype(indices)(std::forward<T>(ind));
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

			for (int i = 0; i < 3; ++i) {
				auto t = tim * glm::vec4(tangent_frame[i], 1);
				tangent_frame[i] = glm::normalize(glm::vec3{ t.x, t.y, t.z });
			}

			auto p = m * glm::vec4(v.p, 1);
			v.p = { p.x,p.y,p.z };
			v.tangent_frame_from_tbn(tangent_frame[0], tangent_frame[1], tangent_frame[2]);
		}
		calc_sphere();
	}
};

}
}
