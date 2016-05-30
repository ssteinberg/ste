
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
layout(binding = 11) uniform sampler2D depth_texture;

const int samples = 2;
const int rings = 5;

const float aperature_distance = 0.5f;

const float blur_coef = 1.0;
const float threshold = 0.5f; 	// highlight threshold
const float gain = 2.f; 		// highlight gain

const float bias = 0.5f; 		// bokeh edge bias
const float fringe = 0.7f; 		// bokeh chromatic aberration/fringing
const float namount = 0.0001f; 	// dither amount

uniform vec2 size;
uniform float aperature_diameter = .02f;

vec2 texel = 1.f / size;

vec3 color(vec2 coords, float blur) {
	vec3 col = vec3(0.0);

	col.r = texture(hdr, coords + vec2(0.f,		1.0f)*texel*fringe*blur).r;
	col.g = texture(hdr, coords + vec2(-.866f, -0.5f)*texel*fringe*blur).g;
	col.b = texture(hdr, coords + vec2(.866f,  -0.5f)*texel*fringe*blur).b;

	vec3 lumcoeff = vec3(0.299f, 0.587f, 0.114f);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum - threshold)*gain, 0.f);
	return col + mix(vec3(0.f), col, thresh*blur);
}

float coc(float z) {
	float focal = params.focus;

	float s = z / 10000.f;

	float C = aperature_diameter * abs(focal - s) / s;
	float c = C * aperature_distance / focal;
	return clamp(smoothstep(0.f, 1.f, c), 0.f, 1.f);
}

void main() {
	vec2 uv = vec2(gl_FragCoord.xy) / size;
	vec3 col = texture(hdr, uv).rgb;

	float d = texelFetch(depth_texture, ivec2(gl_FragCoord.xy), 0).x;
	float z = unproject_depth(d);
    float blur = coc(z);

	if (blur > .01f) {
		vec2 noise = vec2(fast_rand(uv.x), fast_rand(uv.y)) * namount * blur;
		vec2 wh = (1.f / size) * blur * blur_coef + noise;

		float s = 1.f;
		for (int i = 1; i <= rings; ++i) {
			int ringsamples = i * samples;

			for (int j = 0; j < ringsamples; ++j) {
				float step = float(j) * pi*2.f / float(ringsamples);
				vec2 c = vec2(cos(step), sin(step)) * float(i);
				float w = mix(1.f, float(i) / float(rings), bias);

				col += color(uv + c * wh, blur) * w;
				s += w;
			}
		}

		col /= s;
	}

	gl_FragColor = col;
}
