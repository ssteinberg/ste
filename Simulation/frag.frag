
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

struct material_texture_descriptor {
	uint64_t tex_handler;
};
struct material_descriptor {
	material_texture_descriptor diffuse;
	material_texture_descriptor specular;
	material_texture_descriptor heightmap;
	material_texture_descriptor alphamap;
	material_texture_descriptor tex0;
	material_texture_descriptor tex1;
	material_texture_descriptor tex2;
	material_texture_descriptor tex3;
};

mat3 RGBtoXYZ = mat3(0.5767309, 0.2973769, 0.0270343,
					 0.1855540, 0.6273491, 0.0706872, 
					 0.1881852, 0.0752741, 0.9911085);
const int histogram_bins = 64;

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in float frag_depth;
flat in int drawId;

layout(location = 0) out vec3 o_frag_color;
layout(location = 1) out vec3 o_frag_position;
layout(location = 2) out vec3 o_frag_normal;

layout(binding = 0, r32i) uniform iimage2D depth_layer0;
layout(binding = 1, r32i) uniform iimage2D depth_layer1;
layout(binding = 2, r32i) uniform iimage2D depth_layer2;

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

	vec3 color = md.diffuse.tex_handler>0 ? texture(sampler2D(md.diffuse.tex_handler), frag_texcoords).rgb : vec3(1,1,1);
	vec3 XYZ = RGBtoXYZ * color;
	float XYZtotal = XYZ.x + XYZ.y + XYZ.z;
	o_frag_color = vec3(XYZ.xy / XYZtotal, XYZ.y);

	o_frag_position = frag_position;
	
	int int_from_float = floatBitsToInt(1.0f - frag_depth);
	int t;
	if ((t=imageAtomicMax(depth_layer0, ivec2(gl_FragCoord.xy), int_from_float)) < int_from_float) int_from_float = t;
	if ((t=imageAtomicMax(depth_layer1, ivec2(gl_FragCoord.xy), int_from_float)) < int_from_float) int_from_float = t;
	imageAtomicMax(depth_layer2, ivec2(gl_FragCoord.xy), int_from_float);
}
