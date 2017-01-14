
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "common.glsl"
#include "chromaticity.glsl"
#include "hdr_common.glsl"
#include "fast_rand.glsl"

#include "gbuffer.glsl"
#include "girenderer_transform_buffer.glsl"

out vec3 gl_FragColor;

layout(std430, binding = 2) restrict readonly buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

layout(bindless_sampler) uniform sampler2D hdr;
layout(bindless_sampler) uniform sampler2D depth_texture;

const int samples = 4;
const int max_rings = 7;
const float max_blur_texels_radius = 4.f;

const float blur_coef = 1.0;
const float threshold = 0.5f; 	// highlight threshold
const float gain = 2.f; 		// highlight gain

const float bias = 0.5f; 		// bokeh edge bias
const float fringe = 1.f; 		// bokeh chromatic aberration/fringing
const float namount = 0.0001f; 	// dither amount

uniform vec2 size;
uniform float aperture_distance = 17e-3f;	// Defaults to human eye length from retina to pupil, about 17mm
uniform float aperture_diameter = 6e-3f;	// Defaults to human eye pupil diameter which ranges from 2mm to 8mm

vec3 color(vec2 coords, vec2 blur, float max_blur) {
	vec2 jitter = fringe * blur;

	vec3 col;
	col.r = texture(hdr, coords + vec2(0.f,		1.0f) * jitter).r;
	col.g = texture(hdr, coords + vec2(-.866f, -0.5f) * jitter).g;
	col.b = texture(hdr, coords + vec2(.866f,  -0.5f) * jitter).b;

	float lum = luminance(col.rgb);
	float thresh = max((lum - threshold)*gain, 0.f);

	return col + mix(vec3(0.f), col, thresh * max_blur);
}

vec2 coc(float z) {
	float focal = params.focus;
	float s = z;

	// Circle of confusion diameter in world space
	float C = aperture_diameter * abs(focal - s) / (-s);
	// Project onto near clip plane
	vec4 projected = project(vec3(C,0,focal));
	float c = projected.x / projected.w;

	// Normalize to screen texture space
	float aperture_h = 2.f * projection_tan_half_fovy() * projection_near_clip();
	float aperture_w = aperture_h * projection_aspect();
	vec2 norm_coc = c / vec2(aperture_w, aperture_h);

	return .5f * norm_coc;
}

void main() {
	vec2 uv = vec2(gl_FragCoord.xy) / size;
	vec3 col = texture(hdr, uv).rgb;

	float d = texelFetch(depth_texture, ivec2(gl_FragCoord.xy), 0).x;
	float z = unproject_depth(d);
	
	vec2 blur = min(coc(z), max_blur_texels_radius / size);
	vec2 blur_texels = blur * size;
	float max_blur = max_element(blur_texels);
	
	/*int rings = min(int(round(max_blur / max_blur_texels_radius * float(max_rings))), max_rings);
	if (rings > 0) {
		vec2 noise = vec2(fast_rand(uv.x), fast_rand(uv.y)) * namount * blur_texels;
		vec2 wh = blur * blur_coef + noise;

		float s = 1.f;
		for (int i = 1; i <= rings; ++i) {
			int ringsamples = i * samples;

			for (int j = 0; j < ringsamples; ++j) {
				float angle = float(j) * two_pi / float(ringsamples);
				vec2 c = vec2(cos(angle), sin(angle)) * float(i);
				float w = mix(1.f, float(i) / float(rings), bias);

				col += color(uv + c * wh, blur, max_blur) * w;
				s += w;
			}
		}

		col /= s;
	}*/

	gl_FragColor = col;
}
