
#type compute
#version 450

struct bokeh_descriptor {
	vec2 pos;
	float coc;
	vec4 color;
};

layout(local_size_x = 4, local_size_y = 4) in;

layout(binding = 0) uniform sampler2D hdr;

layout(binding = 2, r32f) uniform readonly image2D z_buffer;
layout(binding = 3, r16) uniform writeonly image2D coc_buffer;
layout(binding = 1) uniform atomic_uint counter;

layout(std430, binding = 2) coherent buffer brokeh_buffer {
	bokeh_descriptor brokeh[];
};

uniform float aperature_radius = .25f;
uniform float bokeh_lum_threshold = .2f;
uniform float bokeh_coc_threshold = .2f;

shared float focal;
shared float lum[gl_WorkGroupSize.x * 2][gl_WorkGroupSize.y * 2];

void main() {
	uvec2 id = gl_LocalInvocationID.xy;
	ivec2 texSize = imageSize(z_buffer);
	ivec2 lum_padding = ivec2(gl_WorkGroupSize.xy / 2);

	ivec2 lum_id = ivec2(id) + lum_padding;

	if (id == uvec2(0, 0))
		focal = imageLoad(z_buffer, texSize / 2).x;

	float s = imageLoad(z_buffer, ivec2(gl_GlobalInvocationID.xy)).x;

	{
		uvec2 id2 = id * 2;
		vec4 l_texels = textureGather(hdr, vec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy + id2) / vec2(texSize), 3);

		lum[id2.x][id2.y + 1] = l_texels.x;
		lum[id2.x + 1][id2.y + 1] = l_texels.y;
		lum[id2.x + 1][id2.y] = l_texels.z;
		lum[id2.x][id2.y] = l_texels.w;
	}

	barrier();
	memoryBarrierShared();
	
	float C = aperature_radius * abs(focal - s) / s;
	float coc = clamp(smoothstep(0, 1, C), 0, 1);
	imageStore(coc_buffer, ivec2(gl_GlobalInvocationID.xy), vec4(coc, 0, 0, 0));

	float lum_neigh = .0f;
	for (int x = -lum_padding.x; x <= lum_padding.x; ++x)
		for (int y = -lum_padding.y; y <= lum_padding.y; ++y)
			lum_neigh += lum[lum_id.x + x][lum_id.y + y];
	lum_neigh /= (gl_WorkGroupSize.x + 1) * (gl_WorkGroupSize.y + 1);

	float lum_diff = lum[lum_id.x][lum_id.y] - lum_neigh;
	if (lum_diff >= bokeh_lum_threshold && 
		coc >= bokeh_coc_threshold) {
		vec3 hdr_texel = texelFetch(hdr, ivec2(gl_GlobalInvocationID.xy), 0).rgb;
		
		uint idx = atomicCounterIncrement(counter);
		if (idx < 4096) {
			brokeh[idx].pos = vec2(gl_GlobalInvocationID.xy);
			brokeh[idx].coc = coc;
			brokeh[idx].color = vec4(hdr_texel, 1.f);
		}
	}
}
