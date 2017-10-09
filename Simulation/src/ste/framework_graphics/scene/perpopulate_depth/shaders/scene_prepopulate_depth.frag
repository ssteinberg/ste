
#type frag
#version 450

#include <material.glsl>

layout(location = 0) in scene_transform {
	vec3 frag_position;
	vec3 frag_normal;
	vec3 frag_tangent;
	vec2 frag_texcoords;
	flat int material_id;
} vin;

void main() {
	material_descriptor md = mat_descriptor[vin.material_id];

	if (material_is_masked(md, vin.frag_texcoords))
		discard;
}
