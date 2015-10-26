
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "hdr_common.glsl"

struct material_texture_descriptor {
	uint64_t tex_handler;
};
struct material_descriptor {
	material_texture_descriptor diffuse;
	material_texture_descriptor specular;
	material_texture_descriptor heightmap;
	material_texture_descriptor normalmap;
	material_texture_descriptor alphamap;
};

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_tangent;
in vec3 frag_bitangent;
in float frag_depth;
flat in int drawId;

layout(location = 0) out vec3 o_frag_color;
layout(location = 1) out vec3 o_frag_position;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;
layout(location = 4) out vec3 o_frag_tangent;
layout(location = 5) out float o_frag_specular;

uniform float height_map_scale = 100.0f;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

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
	vec3 b = normalize(frag_bitangent);
	//float h = (textureGrad(sampler2D(md.heightmap.tex_handler), uv, dUVdx, dUVdy).r - 1.0f) * height_map_scale;

	if (md.normalmap.tex_handler>0) {
		vec3 nm = texture(sampler2D(md.normalmap.tex_handler), uv).xyz;
		n = normalize(mat3(t, b, n) * nm);
	}
	
	o_frag_tangent = t;
	o_frag_normal = n;
	
	o_frag_specular = md.specular.tex_handler>0 ? texture(sampler2D(md.specular.tex_handler), uv).r : 1;
	
	vec3 color = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), uv).rgb : vec3(1);
	o_frag_color = XYZtoxyY(RGBtoXYZ(color));

	o_frag_position = P;
	o_frag_z = frag_depth;
}
