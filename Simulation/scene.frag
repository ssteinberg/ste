
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_tangent;
in float frag_depth;
flat in int drawId;

layout(location = 0) out vec3 o_frag_position;
layout(location = 1) out vec4 o_frag_color;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;
layout(location = 4) out vec3 o_frag_tangent;
layout(location = 5) out int o_frag_mat_idx;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

uniform float height_map_scale = 1.f;

void main() {
	material_descriptor md = mat_descriptor[drawId];

	if (md.alphamap.tex_handler>0 && texture(sampler2D(md.alphamap.tex_handler), frag_texcoords).x<.5f) {
		discard;
		return;
	}
	
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;
	vec3 n = normalize(frag_normal);
	vec3 t = normalize(frag_tangent);
	
	if (md.normalmap.tex_handler>0) {
		vec4 normal_height = texture(sampler2D(md.normalmap.tex_handler), uv);
		vec3 b = cross(t, n);
		mat3 tbn = mat3(t, b, n);

		float h = normal_height.w * height_map_scale;
		P += h * n;

		vec3 nm = normal_height.xyz;
		n = tbn * nm;

		t = cross(n, b);
	}
	float specular = md.specular.tex_handler>0 ? texture(sampler2D(md.specular.tex_handler), uv).x : 1.f;
	vec3 diffuse = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv).rgb : vec3(1.f);
	
	o_frag_tangent = t;
	o_frag_normal = n;
	o_frag_color = vec4(diffuse, specular);

	o_frag_position = P;
	o_frag_z = frag_depth;
	o_frag_mat_idx = drawId;
}
