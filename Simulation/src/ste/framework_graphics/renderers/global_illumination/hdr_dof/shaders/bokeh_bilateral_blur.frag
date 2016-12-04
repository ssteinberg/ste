
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

#include "common.glsl"
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

const int samples = 2;
const int max_rings = 5;

const float blur_coef = 1.0;
const float threshold = 0.5f; 	// highlight threshold
const float gain = 2.f; 		// highlight gain

const float bias = 0.5f; 		// bokeh edge bias
const float fringe = 0.7f; 		// bokeh chromatic aberration/fringing
const float namount = 0.0001f; 	// dither amount

uniform vec2 size;
uniform float aperture_distance = 17e-3f;	// Defaults to human eye length from retina to lens, about 17mm
uniform float aperture_diameter = 4e-3f;	// Defaults to human eye pupil diameter which range from 2mm to 8mm

vec3 color(vec2 coords, vec2 blur, float max_blur) {
	vec3 col = vec3(0.0);

	vec2 jitter = fringe * blur;
	col.r = texture(hdr, coords + vec2(0.f,		1.0f) * jitter).r;
	col.g = texture(hdr, coords + vec2(-.866f, -0.5f) * jitter).g;
	col.b = texture(hdr, coords + vec2(.866f,  -0.5f) * jitter).b;

	vec3 lumcoeff = vec3(0.299f, 0.587f, 0.114f);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum - threshold)*gain, 0.f);
	return col + mix(vec3(0.f), col, thresh * max_blur);
}

vec2 coc(float z) {
	float focal = params.focus;

	float s = z;

	// Circle of confusion diameter in world space
	float C = aperture_diameter * abs(focal - s) / s;
	// Circle of confusion size on the screen plane in world units
	float c = C / focal;

	// Normalize to screen texture space
	float aperture_h = 2.f * projection_tan_half_fovy();
	float aperture_w = aperture_h * projection_aspect();
	vec2 norm_coc = c / vec2(aperture_w, aperture_h);

	// We expect radius
	return clamp(.5f * norm_coc, vec2(0.f), vec2(1.f));
}

void main() {
	vec2 uv = vec2(gl_FragCoord.xy) / size;
	vec3 col = texture(hdr, uv).rgb;

	float d = texelFetch(depth_texture, ivec2(gl_FragCoord.xy), 0).x;
	float z = unproject_depth(d);
	vec2 blur = coc(z);
	float max_blur = max(blur.x, blur.y);

	if (max_blur > .01f) {
		vec2 noise = vec2(fast_rand(uv.x), fast_rand(uv.y)) * namount * blur;
		vec2 wh = blur * blur_coef + noise;

		float s = 1.f;
		int rings = min(max_rings, max(2, int(max_blur * 15.f)));
		for (int i = 1; i <= rings; ++i) {
			int ringsamples = i * samples;

			for (int j = 0; j < ringsamples; ++j) {
				float step = float(j) * two_pi / float(ringsamples);
				vec2 c = vec2(cos(step), sin(step)) * float(i);
				float w = mix(1.f, float(i) / float(rings), bias);

				col += color(uv + c * wh, blur, max_blur) * w;
				s += w;
			}
		}

		col /= s;
	}

	gl_FragColor = col;
}
