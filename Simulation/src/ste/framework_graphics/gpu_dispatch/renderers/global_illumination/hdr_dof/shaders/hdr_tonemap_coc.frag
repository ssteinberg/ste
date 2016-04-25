
#type frag
#version 450
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

#include "hdr_common.glsl"
#include "gbuffer.glsl"

layout(location = 0) out vec4 rgbout;
layout(location = 1) out vec4 bloomout;
layout(location = 2) out vec2 coc_out;

layout(std430, binding = 0) coherent buffer histogram_sums {
	uint histogram[bins];
};
layout(std430, binding = 2) coherent readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

const float bloom_cutoff = .9f;

layout(bindless_sampler) uniform sampler2D hdr;
layout(bindless_sampler) uniform sampler1D hdr_vision_properties_texture;

uniform float aperature_radius = .25f;
uniform float f1 = .1f;
uniform float far, near;

void main() {
	vec3 hdr_texel = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0).rgb;

	float min_lum = intBitsToFloat(params.lum_min);
	float max_lum = intBitsToFloat(params.lum_max);

	float l = hdr_lum(hdr_texel.z);
	float fbin = hdr_bin(max_lum, min_lum, l);
	int bin = int(fbin);
	float frac = fract(fbin);

	float T = float(histogram[bins - 1]);
	float toned_bin_start = bin > 0 ? float(histogram[bin - 1]) / T : .0f;
	float toned_bin_end = float(histogram[bin]) / T;
	float toned_bin_size = toned_bin_end - toned_bin_start;

	float toned_l = toned_bin_start + frac * toned_bin_size;

	vec4 vision_properties = texture(hdr_vision_properties_texture, clamp((l - min_luminance) / (10.f - min_luminance), 0.f, 1.f));
	float scotopic = vision_properties.x;
	float mesopic = vision_properties.y;
	float monochr = vision_properties.z;
	float acuity = vision_properties.w;
	float red_coef = red_response(mesopic);

	hdr_texel.z = tonemap(toned_l) * mix(1.f, .666f, scotopic);

	vec3 XYZ = xyYtoXYZ(hdr_texel);
	vec3 RGB = XYZtoRGB(XYZ);
	RGB.r *= red_coef;
	RGB = mix(RGB, XYZ.yyy, monochr);
	vec4 RGBL = clamp(vec4(RGB, XYZ.y), vec4(0.f), vec4(1.f));

	rgbout = RGBL;

	if (XYZ.y > bloom_cutoff) {
		float x = pow((XYZ.y - bloom_cutoff) / (1.f - bloom_cutoff), 8) * (1.f - mesopic);
		bloomout = vec4(RGBL.rgb, x);
	}
	else
		bloomout = vec4(0);

	float focal = params.focus;

	g_buffer_element frag = gbuffer_load(ivec2(gl_FragCoord.xy));
	float s = gbuffer_linear_z(frag, far, near);

	float C = aperature_radius * abs(focal - s) / s;
	float c = C * f1 / focal;
	float coc = clamp(smoothstep(0.f, 1.f, c), 0.f, 1.f);
	coc += acuity;

	coc_out = vec2(s, clamp(coc, 0.f, 1.f));
}
