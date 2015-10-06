
#version 450
#extension GL_ARB_compute_variable_group_size : require

layout(local_size_variable) in;
layout(binding = 0, rgba32f) uniform readonly image2D tex;
layout(binding = 1, rgba32f) uniform writeonly image2D otex;

void main() {
	vec4 a = imageLoad(tex, ivec2(gl_WorkGroupID.xy) * 2);
	vec4 b = imageLoad(tex, ivec2(gl_WorkGroupID.xy) * 2 + ivec2(1,0));
	vec4 c = imageLoad(tex, ivec2(gl_WorkGroupID.xy) * 2 + ivec2(0,1));
	vec4 d = imageLoad(tex, ivec2(gl_WorkGroupID.xy) * 2 + ivec2(1,1));
	
	float lum_max = max(max(a.z, b.z), max(c.z, d.z));
	float lum_min = min(min(a.x, b.x), min(c.x, d.x));
	float e = a.y + b.y + c.y + d.y;

	imageStore(otex, ivec2(gl_WorkGroupID.xy), vec4(lum_min, e * .25f, lum_max, 1));
}
