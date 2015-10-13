
#type frag
#version 440

out float gl_FragColor;

layout(binding = 0) uniform sampler2D occlusion_tex;
layout(binding = 1) uniform sampler2D position_tex;

const float center_weight = 0.20236;
const vec4 weights = vec4(0.028532, 0.067234, 0.124009, 0.179044);
const int offsets[4] = { -6, -4, -3, -1 };
const vec4 one = float(1).xxxx;
const float depth_tolerance = 5;

void main() {
	vec2 tex_inv_size = 1.0f / textureSize(occlusion_tex, 0);
	vec2 tex_coords = gl_FragCoord.xy * tex_inv_size;

	float occ = textureLod(occlusion_tex, tex_coords, 0).r;
	float depth = textureLod(position_tex, tex_coords, 0).z;

	float result = occ * center_weight;
	float total = center_weight;
	
	const ivec2 dir = ivec2(0,1);
	const ivec2 a[4] = {dir * offsets[0],dir * offsets[1],dir * offsets[2],dir * offsets[3]};
	const ivec2 b[4] = {-dir * offsets[3],-dir * offsets[2],-dir * offsets[1],-dir * offsets[0]};
	vec4 occ_texels_a = textureGatherOffsets(occlusion_tex, tex_coords,  a, 0);
	vec4 depth_texels_a = textureGatherOffsets(position_tex, tex_coords, a, 2);
	vec4 occ_texels_b = textureGatherOffsets(occlusion_tex, tex_coords, b, 0);
	vec4 depth_texels_b = textureGatherOffsets(position_tex, tex_coords, b, 2);

	vec4 diff_a = abs(depth_texels_a - depth.xxxx);
	vec4 w_a = one.xxxx - smoothstep(float(0).xxxx, depth_tolerance.xxxx, diff_a);
	w_a *= weights;

	vec4 diff_b = abs(depth_texels_b - depth.xxxx) + one;
	vec4 w_b = one.xxxx - smoothstep(float(0).xxxx, depth_tolerance.xxxx, diff_b);
	w_b *= weights;

	result += dot(w_a, occ_texels_a) + dot(w_b, occ_texels_b);
	total += dot(w_a, one) + dot(w_b, one);
	result /= total;

	gl_FragColor = result;
}
