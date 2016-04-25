
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require
// #extension GL_NV_shader_atomic_fp16_vector : require

const int light_buffers_first = 2;

#include "material.glsl"
#include "light.glsl"
#include "gbuffer.glsl"
#include "shadow.glsl"
//#include "voxels.glsl"

in vec2 tex_coords;

out vec4 gl_FragColor;

layout(std430, binding = 5) buffer projection_data {
	mat4 shadow_transforms[];
};

layout(binding = 8) uniform samplerCubeArrayShadow shadow_depth_maps;

uniform float scattering_ro = 0.0003f;
uniform mat4 inverse_view_matrix;
uniform float proj22, proj23;

vec4 shade(g_buffer_element frag) {
	uint16_t draw_idx = frag.material;
	vec4 c = frag.albedo;

	vec3 diffuse = c.rgb;
	float alpha = c.a;
	float specular = mix(.3f, 1.f, frag.specular);

	if (draw_idx == material_none)
		return vec4(diffuse, alpha);

	vec3 n = frag.N;
	vec3 t = frag.T;
	vec3 b = cross(t, n);
	vec3 position = frag.P.xyz;
	vec3 w_pos = (inverse_view_matrix * vec4(position, 1)).xyz;

	material_descriptor md = mat_descriptor[draw_idx];

	vec3 rgb = md.emission.rgb;
	for (int i = 0; i < light_buffer.length(); ++i) {
		light_descriptor ld = light_buffer[i];

		vec3 v = light_incidant_ray(ld, i, position);
		if (dot(n, v) <= 0)
			continue;

		float dist = length(v);
		float l_radius = light_buffer[i].radius;
		vec3 l = diffuse * ld.diffuse.xyz;

		vec3 shadow_v = w_pos - light_buffer[i].position_direction.xyz;
		float shadow = shadow_penumbra_width(shadow_depth_maps, i, shadow_v, l_radius, dist, proj22, proj23);

		float dist_att = dist * scattering_ro;
		float shadow_attenuation = 1.f - exp(-dist_att * dist_att);
		float obscurance = mix(1.f, .3f * shadow_attenuation, shadow);

		float brdf = calc_brdf(md, position, n, t, b, v);
		float attenuation_factor = light_attenuation_factor(ld, dist);
		float incident_radiance = ld.luminance / attenuation_factor;

		float irradiance = specular * brdf * incident_radiance * obscurance;
		rgb += l * max(0.f, irradiance);
	}

	return vec4(rgb, alpha);
}

void main() {
	g_buffer_element frag = gbuffer_load(ivec2(gl_FragCoord.xy));
	vec4 c = shade(frag);

	int i = 0;
	while (!gbuffer_eof(frag.next_ptr) && i++ < 5) {
		frag = gbuffer_load(frag.next_ptr);
		vec4 c2 = shade(frag);

		c = c * c.a + c2 * (1.f - c.a);
	}

	vec3 xyY = XYZtoxyY(RGBtoXYZ(c.rgb));
	xyY.z = max(min_luminance, xyY.z);

	gl_FragColor = vec4(xyY, 1);


	// vec2 p = gl_FragCoord.xy / vec2(1688, 950);
	// p = p * 2 - vec2(1);
	// vec3 D = normalize((inv_view_model * inv_projection * vec4(p, 0, 1)).xyz);
	// vec3 P = vec3(0);
	// float radius = 0;

	// vec3 normal;
	// vec4 color;

	// bool hit;
	// float ray_length;

	// P = voxel_cone_march(P, D, vec3(0), radius, 0.06, radius, hit, ray_length);
	// //P = voxel_ray_march(P, D, vec3(0), hit, ray_length);

	// voxel_filter(P, radius, color, normal);

	// xyY = XYZtoxyY(RGBtoXYZ(vec3(ray_length / 100.f)));
	// xyY.z *= 1000;
	// gl_FragColor = color;
}
