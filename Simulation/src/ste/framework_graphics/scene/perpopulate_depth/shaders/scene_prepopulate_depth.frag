
#type frag
#version 450

#include <material.glsl>

in scene_transform {
	vec3 frag_position;
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int matIdx;
} vin;

void main() {
	material_descriptor md = mat_descriptor[vin.matIdx];

	if (material_is_masked(md, vin.frag_texcoords))
		discard;
}
