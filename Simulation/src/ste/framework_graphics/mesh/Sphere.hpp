// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <mesh.hpp>

namespace ste {
namespace graphics {

class sphere : public mesh<mesh_subdivion_mode::Triangles> {
public:
	~sphere() noexcept override {}

	sphere(unsigned vert_slices, unsigned horz_slices, float theta_min = -glm::half_pi<float>(), float theta_max = glm::half_pi<float>()) {
		lib::vector<object_vertex_data> vert;
		lib::vector<std::uint32_t> ind;

		for (unsigned x = 0; x <= horz_slices; ++x) {
			float phi = x == horz_slices ? -glm::pi<float>() : glm::mix(-glm::pi<float>(), glm::pi<float>(), static_cast<float>(x) / static_cast<float>(horz_slices - 1));
			for (unsigned y = 0; y < vert_slices; ++y) {
				float theta = glm::mix(theta_min, theta_max, static_cast<float>(y) / static_cast<float>(vert_slices - 1));

				object_vertex_data v;
				v.p.y = glm::sin(theta);
				v.p.x = glm::cos(theta) * glm::sin(phi);
				v.p.z = glm::cos(theta) * glm::cos(phi);

				glm::vec3 n = v.p;
				glm::vec3 t;
				t.y = glm::sin(theta + glm::half_pi<float>());
				t.x = glm::cos(theta + glm::half_pi<float>()) * glm::sin(phi);
				t.z = glm::cos(theta + glm::half_pi<float>()) * glm::cos(phi);
				glm::vec3 b = glm::cross(t,n);
				v.tangent_frame_from_tbn(t,b,n);

				v.uv.x = .5f + glm::atan(v.p.x, v.p.z) / glm::two_pi<float>();
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
				ind.push_back(i1);
				ind.push_back(i2);
				ind.push_back(i2);
				ind.push_back(i1);
				ind.push_back(i3);
			}
		}

		set_vertices(vert);
		set_indices(ind);
	}
};

}
}
