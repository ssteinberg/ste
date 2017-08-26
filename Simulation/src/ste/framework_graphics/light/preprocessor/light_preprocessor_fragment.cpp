
#include <stdafx.hpp>
#include <light_preprocessor_fragment.hpp>

#include <extract_projection_planes.hpp>

using namespace ste;
using namespace ste::graphics;

void light_preprocessor_fragment::set_projection_planes() const {
	// TODO
	glm::vec4 np;
	glm::vec4 fp;
	glm::vec4 rp;
	glm::vec4 lp;
	glm::vec4 tp;
	glm::vec4 bp;

//	float aspect = ctx.get_projection_aspect();
//	float fovy = ctx.get_fov();
//	float fnear = ctx.get_near_clip();
//
//	extract_projection_frustum_planes(fnear * 2, fnear, fovy, aspect,
//									  &np, &fp, &rp, &lp, &tp, &bp);
//
//	light_preprocess_cull_lights_program.get().set_uniform("np", np);
//	light_preprocess_cull_lights_program.get().set_uniform("rp", rp);
//	light_preprocess_cull_lights_program.get().set_uniform("lp", lp);
//	light_preprocess_cull_lights_program.get().set_uniform("tp", tp);
//	light_preprocess_cull_lights_program.get().set_uniform("bp", bp);
}
