
#type frag
#version 450

layout(set=0, binding=1) uniform sampler2D microfacet_refraction_fit_lut;
layout(set=0, binding=2) uniform sampler2DArray microfacet_transmission_fit_lut;

layout(set=0, binding=3) uniform sampler2D ltc_ggx_fit;
layout(set=0, binding=4) uniform sampler2D ltc_ggx_amplitude;

#include <chromaticity.glsl>

#include <light.glsl>
//#include <light_cascades.glsl>
#include <linked_light_lists.glsl>
#include <linearly_transformed_cosines.glsl>

#include <gbuffer.glsl>
//#include <shadow.glsl>

#include <material.glsl>
#include <material_evaluate.glsl>

#include <deferred_shading.glsl>

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 frag_color;

g_buffer_element read_gbuffer(ivec2 coords) {
	g_buffer_element g_frag;

	g_frag.data[0] = texelFetch(gbuffer, ivec3(coords, 0), 0);
	g_frag.data[1] = texelFetch(gbuffer, ivec3(coords, 1), 0);
	
	return g_frag;
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);

	g_buffer_element g_frag = read_gbuffer(coord);
	vec3 shaded_fragment;// = deferred_shade_fragment(g_frag, coord);
	
	vec3 P = eye_position();

	vec3 V0 = transform_view_to_world_space(unproject_screen_position(.5f, (vec2( .5f, .5f) + coord) / vec2(backbuffer_size()))) - P;
	V0 = V0 == vec3(0) ? vec3(1,0,0) : normalize(V0);
	voxel_traversal_result_t ret0 = voxel_traverse_ray(P, V0, 150);

	float dist = ret0.distance;

	shaded_fragment = isinf(dist) ? vec3(10000,0,0) : ret0.data.albedo.rgb * 25.f;
	
	vec3 xyY = XYZtoxyY(RGBtoXYZ(shaded_fragment));
	frag_color = vec4(xyY, 1);
}
