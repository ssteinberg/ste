
#include "stdafx.hpp"
#include "light_preprocessor.hpp"

#include "extract_projection_planes.hpp"

using namespace StE::Graphics;

void light_preprocessor::set_projection_planes() const {
	glm::vec4 np;
	glm::vec4 fp;
	glm::vec4 rp;
	glm::vec4 lp;
	glm::vec4 tp;
	glm::vec4 bp;

	float aspect = ctx.get_projection_aspect();
	float fovy = ctx.get_fov();
	float near = ctx.get_near_clip();

	extract_projection_frustum_planes(near * 2, near, fovy, aspect,
									  &np, &fp, &rp, &lp, &tp, &bp);

	light_preprocess_cull_lights_program->set_uniform("np", np);
	light_preprocess_cull_lights_program->set_uniform("rp", rp);
	light_preprocess_cull_lights_program->set_uniform("lp", lp);
	light_preprocess_cull_lights_program->set_uniform("tp", tp);
	light_preprocess_cull_lights_program->set_uniform("bp", bp);
	light_preprocess_cull_shadows_program->set_uniform("np", np);
	light_preprocess_cull_shadows_program->set_uniform("rp", rp);
	light_preprocess_cull_shadows_program->set_uniform("lp", lp);
	light_preprocess_cull_shadows_program->set_uniform("tp", tp);
	light_preprocess_cull_shadows_program->set_uniform("bp", bp);
}
