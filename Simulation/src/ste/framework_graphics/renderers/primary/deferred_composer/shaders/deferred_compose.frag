
#type frag
#version 450

#include <chromaticity.glsl>

#include <light.glsl>
#include <light_cascades.glsl>
#include <linked_light_lists.glsl>
#include <linearly_transformed_cosines.glsl>

#include <gbuffer.glsl>
#include <shadow.glsl>

#include <material.glsl>
#include <material_evaluate.glsl>

layout(location = 7) uniform sampler2D microfacet_refraction_fit_lut;
layout(location = 8) uniform sampler2DArray microfacet_transmission_fit_lut;

layout(location = 9) uniform sampler2D ltc_ggx_fit;
layout(location = 10) uniform sampler2D ltc_ggx_amplitude;

layout(location = 11) uniform sampler3D scattering_volume;

layout(location = 12) uniform sampler2DArray atmospheric_optical_length_lut;
layout(location = 13) uniform sampler3D atmospheric_scattering_lut;
layout(location = 14) uniform sampler3D atmospheric_mie0_scattering_lut;
layout(location = 15) uniform sampler3D atmospheric_ambient_lut;

#include <deferred_shading.glsl>

out vec4 frag_color;

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
