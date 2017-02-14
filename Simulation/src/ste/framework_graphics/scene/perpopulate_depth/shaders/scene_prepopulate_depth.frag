
#type frag
#version 450
#extension GL_ARB_bindless_texture : enable

#include <material.glsl>

layout(std430, binding = 13) restrict readonly buffer material_data {
	material_descriptor mat_descriptor[];
};

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
