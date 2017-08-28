// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <mesh.hpp>

namespace ste {
namespace graphics {

class quad : public mesh<mesh_subdivion_mode::TrianglesStrip> {
public:
	~quad() noexcept override {}
	quad() {
		lib::vector<object_vertex_data> vert;
		object_vertex_data vd;
		glm::vec3 n = { 0,0,1 };
		glm::vec3 t = { 1,0,0 };
		glm::vec3 b = { 0,1,0 };

		vd.tangent_frame_from_tbn(t,b,n);

		vd.p = { -1, -1, 0 };
		vd.uv = { 0,0 };
		vert.push_back(vd);

		vd.p = { 1, -1, 0 };
		vd.uv = { 1,0 };
		vert.push_back(vd);

		vd.p = { -1, 1, 0 };
		vd.uv = { 0,1 };
		vert.push_back(vd);

		vd.p = { 1, 1, 0 };
		vd.uv = { 1,1 };
		vert.push_back(vd);

		set_vertices(vert);
		set_indices(lib::vector<std::uint32_t>{ 0,1,2,3 });
	}
};

extern quad screen_filling_quad;

}
}
