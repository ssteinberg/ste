
#include <stdafx.hpp>
#include <light_preprocessor_fragment.hpp>

#include <extract_projection_planes.hpp>
#include <std140.hpp>

using namespace ste;
using namespace ste::graphics;

void light_preprocessor_fragment::update_projection_planes(const primary_renderer_camera::projection_model_t &camera_projection) {
	using clip_planes_uniform = gl::std140<glm::vec4, glm::vec4, glm::vec4, glm::vec4, glm::vec4>;

	glm::vec4 fp;
	glm::vec4 np;
	glm::vec4 rp;
	glm::vec4 lp;
	glm::vec4 tp;
	glm::vec4 bp;

	// Extract frustum planes
	const float aspect = camera_projection.get_aspect();
	const float fovy = camera_projection.get_fovy();
	const float fnear = static_cast<float>(camera_projection.get_near_clip_plane());
	extract_projection_frustum_planes(fnear * 2, fnear, fovy, aspect,
									  &np, &fp, &rp, &lp, &tp, &bp);

	// Create unifrom struct
	clip_planes_uniform clip_planes;
	clip_planes.get<0>() = np;
	clip_planes.get<1>() = rp;
	clip_planes.get<2>() = lp;
	clip_planes.get<3>() = tp;
	clip_planes.get<4>() = bp;

	pipeline()["push_t.clip_planes"] = clip_planes;
}
