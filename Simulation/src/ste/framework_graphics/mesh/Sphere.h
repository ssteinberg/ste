// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "mesh.h"

namespace StE {
namespace Graphics {

class Sphere : public mesh<mesh_subdivion_mode::Triangles> {
public:
	~Sphere() noexcept override {}

	Sphere(unsigned vert_slices, unsigned horz_slices, float theta_min = -glm::half_pi<float>(), float theta_max = glm::half_pi<float>()) {
		std::vector<ObjectVertexData> vert;
		std::vector<std::uint32_t> ind;

		for (unsigned x = 0; x <= horz_slices; ++x) {
			float phi = x == horz_slices ? -glm::pi<float>() : glm::mix(-glm::pi<float>(), glm::pi<float>(), static_cast<float>(x) / static_cast<float>(horz_slices - 1));
			for (unsigned y = 0; y < vert_slices; ++y) {
				float theta = glm::mix(theta_min, theta_max, static_cast<float>(y) / static_cast<float>(vert_slices - 1));

				ObjectVertexData v;
				v.p.y = glm::sin(theta);
				v.p.x = glm::cos(theta) * glm::sin(phi);
				v.p.z = glm::cos(theta) * glm::cos(phi);
				v.n = v.p;
				v.t.y = glm::sin(theta + glm::half_pi<float>());
				v.t.x = glm::cos(theta + glm::half_pi<float>()) * glm::sin(phi);
				v.t.z = glm::cos(theta + glm::half_pi<float>()) * glm::cos(phi);

				v.uv.x = .5f + glm::atan(-v.p.z, -v.p.x) / glm::two_pi<float>();
				v.uv.y = static_cast<float>(y) / static_cast<float>(vert_slices - 1);

				vert.push_back(v);
			}
		}

		for (unsigned x = 0; x < horz_slices; ++x) {
			for (unsigned y = 0; y < vert_slices - 1; ++y) {
				std::uint32_t i0 = x * vert_slices + y;
				std::uint32_t i1 = (x + 1) * vert_slices + y;
				std::uint32_t i2 = x * vert_slices + y + 1;
				std::uint32_t i3 = (x + 1) * vert_slices + y + 1;

				ind.push_back(i0);
				ind.push_back(i2);
				ind.push_back(i1);
				ind.push_back(i1);
				ind.push_back(i2);
				ind.push_back(i3);
			}
		}

		set_vertices(vert);
		set_indices(ind);
	}
};

}
}
