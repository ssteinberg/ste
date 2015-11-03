// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "mesh.h"

namespace StE {
namespace Graphics {

class Sphere : public mesh<mesh_subdivion_mode::Triangles> {
public:
	~Sphere() noexcept override {}

	Sphere(unsigned vert_slices, unsigned horz_slices, float theta_min = -M_PI/2, float theta_max = M_PI/2) {
		for (unsigned x = 0; x < horz_slices; ++x) {
			float theta = glm::mix(theta_min, theta_max, static_cast<float>(x) / static_cast<float>(horz_slices - 1));
			for (unsigned y = 0; y < vert_slices; ++y) {
				float phi = glm::mix(-M_PI, M_PI, static_cast<float>(y) / static_cast<float>(vert_slices - 1));

				ObjectVertexData v;
				v.p.y = glm::sin(theta);
				v.p.x = glm::cos(theta) * glm::sin(phi);
				v.p.z = glm::cos(theta) * glm::cos(phi);
				v.n = v.p;
				v.t.y = glm::sin(theta + M_PI_2);
				v.t.x = glm::cos(theta + M_PI_2) * glm::sin(phi);
				v.t.z = glm::cos(theta + M_PI_2) * glm::cos(phi);
				v.uv.x = static_cast<float>(x) / static_cast<float>(horz_slices - 1);
				v.uv.y = static_cast<float>(y) / static_cast<float>(vert_slices - 1);

				vertices.push_back(v);
			}
		}

		for (unsigned x = 0; x < horz_slices; ++x) {
			for (unsigned y = 0; y < vert_slices - 1; ++y) {
				unsigned x1 = x < horz_slices - 1 ? x + 1 : 0;

				unsigned i0 = x * vert_slices + y;
				unsigned i1 = (x + 1) * vert_slices + y;
				unsigned i2 = x * vert_slices + y + 1;
				unsigned i3 = (x + 1) * vert_slices + y + 1;

				indices.push_back(i0);
				indices.push_back(i2);
				indices.push_back(i1);
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}
	}
};

}
}
