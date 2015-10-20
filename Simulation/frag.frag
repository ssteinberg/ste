
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
	material_texture_descriptor alphamap;
};

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in float frag_depth;
flat in int drawId;

layout(location = 0) out vec3 o_frag_color;
layout(location = 1) out vec3 o_frag_position;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

void main() {
	material_descriptor md = mat_descriptor[drawId];

	if (md.alphamap.tex_handler>0 && texture(sampler2D(md.alphamap.tex_handler), frag_texcoords).x<.5f) {
		discard;
		return;
	}

	if (md.heightmap.tex_handler>0) {
		vec3 nm = texture(sampler2D(md.heightmap.tex_handler), frag_texcoords).rgb * 2.0f - float(1.0f).xxx;
		nm.y = -nm.y;
		
		vec3 dPdx = dFdx(frag_position);
		vec3 dPdy = dFdy(frag_position);
		vec2 dUVdx = dFdx(frag_texcoords);
		vec2 dUVdy = dFdy(frag_texcoords);
		
		vec3 T = dPdx * dUVdx.x + dPdy * dUVdy.x;
		vec3 B = dPdx * dUVdx.y + dPdy * dUVdy.y;
		float invmax = inversesqrt(max(dot(T,T),dot(B,B)));
		mat3 TBN = mat3(T * invmax, B * invmax, normalize(frag_normal));

		o_frag_normal = normalize(TBN * nm);
	}
	else {
		o_frag_normal = normalize(frag_normal);
	}

	vec3 color = vec3(1);//md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), frag_texcoords).rgb : vec3(1,1,1);
	vec3 XYZ = RGBtoXYZ * color;
	float XYZtotal = XYZ.x + XYZ.y + XYZ.z;
	o_frag_color = vec3(XYZ.xy / XYZtotal, XYZ.y);

	o_frag_position = frag_position;
	o_frag_z = frag_depth;
}
