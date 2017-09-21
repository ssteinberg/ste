
#type frag
#version 450

#include <common.glsl>
#include <hdr_common.glsl>

layout(binding = 0) uniform sampler2D hdr;

layout(std430, binding = 2) restrict buffer hdr_bokeh_parameters_buffer {
	hdr_bokeh_parameters params;
};

layout(location = 0) out float out_lum;

void main() {
	vec2 ts = textureSize(hdr, 0);
	ivec2 coord = ivec2(gl_FragCoord.xy);

	// Read 4x4 texels' luminance values
	vec4 lums0 = textureGatherOffset(hdr, (vec2(coord) * 4 + vec2(.25)) / ts, ivec2(0,0), 2);
	vec4 lums1 = textureGatherOffset(hdr, (vec2(coord) * 4 + vec2(.25)) / ts, ivec2(2,0), 2);
	vec4 lums2 = textureGatherOffset(hdr, (vec2(coord) * 4 + vec2(.25)) / ts, ivec2(2,2), 2);
	vec4 lums3 = textureGatherOffset(hdr, (vec2(coord) * 4 + vec2(.25)) / ts, ivec2(0,2), 2);
	
	// Take maximum, minimum and average
	float max_lum = max_element(vec4(max_element(lums0),
									 max_element(lums1), 
									 max_element(lums2), 
									 max_element(lums3)));
	float min_lum = min_element(vec4(min_element(lums0), 
									 min_element(lums1), 
									 min_element(lums2), 
									 min_element(lums3)));
	vec4 t = vec4(.25f);
	float lum = dot(vec4(dot(lums0,t),
						 dot(lums1,t),
						 dot(lums2,t),
						 dot(lums3,t)), 
					t);

	// Create luminance value from average
	out_lum = hdr_lum(max(min_luminance, lum));

	// Update min and max luminance bounds
	atomicMax(params.lum_max, luminance_to_hdr_params_lum(max_lum));
	atomicMin(params.lum_min, luminance_to_hdr_params_lum(min_lum));
}
