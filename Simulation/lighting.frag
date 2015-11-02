
#type frag
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "material.glsl"

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 3) uniform sampler2D color_tex;
layout(binding = 4) uniform sampler2D tangent_tex;
layout(binding = 5) uniform usampler2D mat_idx_tex;

uniform vec3 light_diffuse;
uniform float light_luminance;
uniform float light_radius = 5;
uniform vec3 light_pos;

layout(std430, binding = 0) buffer material_data {
	material_descriptor mat_descriptor[];
};

void main() {
	uint draw_idx = texelFetch(mat_idx_tex, ivec2(gl_FragCoord.xy), 0).x;
	vec3 normal = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 tangent = texelFetch(tangent_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec3 bitangent = cross(tangent, normal);
	tangent = cross(normal, bitangent);
	vec3 position = texelFetch(position_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	vec4 color_spec = texelFetch(color_tex, ivec2(gl_FragCoord.xy), 0);
	float specular = color_spec.a;
	vec3 color = color_spec.rgb;
	material_descriptor md = mat_descriptor[draw_idx];

	vec3 v = light_pos - position;
	float dist = length(v);

	float brdf = calc_brdf(md, position, normal, tangent, bitangent, v);
	float attenuation_factor = max(.01f, dist / (light_radius * 100));
	float incident_radiance = light_luminance / pow(attenuation_factor, 2);
	
	color *= light_diffuse;
	vec3 xyY = XYZtoxyY(RGBtoXYZ(color));
	xyY.z += min_luminance;
	xyY.z *= max(0, mix(0.5f, 1.f, specular) * brdf * incident_radiance);

	gl_FragColor = vec4(xyY, 1);
}
