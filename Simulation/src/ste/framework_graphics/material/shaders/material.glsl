
#include "common.glsl"

struct material_texture_descriptor {
	uint64_t tex_handler;
};
struct brdf_descriptor {
	uint64_t tex_handler;
	int min_theta_in, max_theta_in;
};
struct material_descriptor {
	material_texture_descriptor diffuse;
	material_texture_descriptor specular;
	material_texture_descriptor normalmap;
	material_texture_descriptor alphamap;
	brdf_descriptor brdf;
	vec4 emission;
};

const int material_none = 0xFFFFFFFF;

void normal_map(material_descriptor md, float height_map_scale, vec2 uv, vec2 duvdx, vec2 duvdy, inout vec3 n, inout vec3 t, inout vec3 b, inout vec3 P) {
	if (md.normalmap.tex_handler > 0) {
		vec4 normal_height = textureGrad(sampler2D(md.normalmap.tex_handler), uv, duvdx, duvdy);
		mat3 tbn = mat3(t, b, n);

		float h = normal_height.w * height_map_scale;
		P += h * n;

		vec3 nm = normal_height.xyz;
		n = tbn * nm;

		t = cross(n, b);
		b = cross(t, n);
	}
}
