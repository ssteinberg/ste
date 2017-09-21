
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
	vec3 shaded_fragment = deferred_shade_fragment(g_frag, coord);

	vec3 xyY = XYZtoxyY(RGBtoXYZ(shaded_fragment));
	frag_color = vec4(xyY, 1);
}
