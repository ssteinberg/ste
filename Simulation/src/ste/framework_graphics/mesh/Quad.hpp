// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "mesh.hpp"

namespace StE {
namespace Graphics {

class Quad : public mesh<mesh_subdivion_mode::TrianglesStrip> {
public:
	~Quad() noexcept override {}
	Quad() {
		std::vector<ObjectVertexData> vert;
		ObjectVertexData vd;
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
		set_indices(std::vector<std::uint32_t>{ 0,1,2,3 });
	}
};

extern Quad ScreenFillingQuad;

}
}
