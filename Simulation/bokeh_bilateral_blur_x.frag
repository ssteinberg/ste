
#type frag
#version 440

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 1) uniform sampler2D zcoc_buffer;

const float center_weight = 0.088959;
const vec4 weights2 = vec4(0.060023, 0.071298, 0.080625, 0.086798);
const vec4 weights1 = vec4(0.018438, 0.026663, 0.036706, 0.048107);
const vec4 weights0 = vec4(0.002578, 0.004539, 0.007608, 0.012138);

void main() {
	vec4 l11 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(12,0), 0);
	vec4 l10 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(11,0), 0);
	vec4 l9 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(10,0), 0);
	vec4 l8 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(9,0), 0);
	vec4 l7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(8,0), 0);
	vec4 l6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(7,0), 0);
	vec4 l5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(6,0), 0);
	vec4 l4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(5,0), 0);
	vec4 l3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(4,0), 0);
	vec4 l2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(3,0), 0);
	vec4 l1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(2,0), 0);
	vec4 l0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(1,0), 0);
	vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	vec4 r0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(1,0), 0);
	vec4 r1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(2,0), 0);
	vec4 r2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(3,0), 0);
	vec4 r3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(4,0), 0);
	vec4 r4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(5,0), 0);
	vec4 r5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(6,0), 0);
	vec4 r6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(7,0), 0);
	vec4 r7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(8,0), 0);
	vec4 r8 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(9,0), 0);
	vec4 r9 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(10,0), 0);
	vec4 r10 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(11,0), 0);
	vec4 r11 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(12,0), 0);
	
	vec2 z_l11 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(12,0), 0).rg;
	vec2 z_l10 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(11,0), 0).rg;
	vec2 z_l9 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(10,0), 0).rg;
	vec2 z_l8 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(9,0), 0).rg;
	vec2 z_l7 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(8,0), 0).rg;
	vec2 z_l6 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(7,0), 0).rg;
	vec2 z_l5 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(6,0), 0).rg;
	vec2 z_l4 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(5,0), 0).rg;
	vec2 z_l3 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(4,0), 0).rg;
	vec2 z_l2 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(3,0), 0).rg;
	vec2 z_l1 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(2,0), 0).rg;
	vec2 z_l0 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - ivec2(1,0), 0).rg;
	float z_c = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy), 0).r;
	vec2 z_r0 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(1,0), 0).rg;
	vec2 z_r1 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(2,0), 0).rg;
	vec2 z_r2 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(3,0), 0).rg;
	vec2 z_r3 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(4,0), 0).rg;
	vec2 z_r4 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(5,0), 0).rg;
	vec2 z_r5 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(6,0), 0).rg;
	vec2 z_r6 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(7,0), 0).rg;
	vec2 z_r7 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(8,0), 0).rg;
	vec2 z_r8 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(9,0), 0).rg;
	vec2 z_r9 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(10,0), 0).rg;
	vec2 z_r10 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(11,0), 0).rg;
	vec2 z_r11 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + ivec2(12,0), 0).rg;
	
	float l11w = clamp(mix(1, 0, (z_l11.x - z_c) / .04f), 0, 1) * l11.w * z_l11.y;
	float l10w = clamp(mix(1, 0, (z_l10.x - z_c) / .04f), 0, 1) * l10.w * z_l10.y;
	float l9w = clamp(mix(1, 0, (z_l9.x - z_c) / .04f), 0, 1) * l9.w * z_l9.y;
	float l8w = clamp(mix(1, 0, (z_l8.x - z_c) / .04f), 0, 1) * l8.w * z_l8.y;
	float l7w = clamp(mix(1, 0, (z_l7.x - z_c) / .04f), 0, 1) * l7.w * z_l7.y;
	float l6w = clamp(mix(1, 0, (z_l6.x - z_c) / .04f), 0, 1) * l6.w * z_l6.y;
	float l5w = clamp(mix(1, 0, (z_l5.x - z_c) / .04f), 0, 1) * l5.w * z_l5.y;
	float l4w = clamp(mix(1, 0, (z_l4.x - z_c) / .04f), 0, 1) * l4.w * z_l4.y;
	float l3w = clamp(mix(1, 0, (z_l3.x - z_c) / .04f), 0, 1) * l3.w * z_l3.y;
	float l2w = clamp(mix(1, 0, (z_l2.x - z_c) / .04f), 0, 1) * l2.w * z_l2.y;
	float l1w = clamp(mix(1, 0, (z_l1.x - z_c) / .04f), 0, 1) * l1.w * z_l1.y;
	float l0w = clamp(mix(1, 0, (z_l0.x - z_c) / .04f), 0, 1) * l0.w * z_l0.y;
	float r0w = clamp(mix(1, 0, (z_r0.x - z_c) / .04f), 0, 1) * r0.w * z_r0.y;
	float r1w = clamp(mix(1, 0, (z_r1.x - z_c) / .04f), 0, 1) * r1.w * z_r1.y;
	float r2w = clamp(mix(1, 0, (z_r2.x - z_c) / .04f), 0, 1) * r2.w * z_r2.y;
	float r3w = clamp(mix(1, 0, (z_r3.x - z_c) / .04f), 0, 1) * r3.w * z_r3.y;
	float r4w = clamp(mix(1, 0, (z_r4.x - z_c) / .04f), 0, 1) * r4.w * z_r4.y;
	float r5w = clamp(mix(1, 0, (z_r5.x - z_c) / .04f), 0, 1) * r5.w * z_r5.y;
	float r6w = clamp(mix(1, 0, (z_r6.x - z_c) / .04f), 0, 1) * r6.w * z_r6.y;
	float r7w = clamp(mix(1, 0, (z_r7.x - z_c) / .04f), 0, 1) * r7.w * z_r7.y;
	float r8w = clamp(mix(1, 0, (z_r8.x - z_c) / .04f), 0, 1) * r8.w * z_r8.y;
	float r9w = clamp(mix(1, 0, (z_r9.x - z_c) / .04f), 0, 1) * r9.w * z_r9.y;
	float r10w = clamp(mix(1, 0, (z_r10.x - z_c) / .04f), 0, 1) * r10.w * z_r10.y;
	float r11w = clamp(mix(1, 0, (z_r11.x - z_c) / .04f), 0, 1) * r11.w * z_r11.y;
			
	vec4 lw0 = vec4(l3w, l2w, l1w, l0w) * weights0;
	vec4 rw0 = vec4(r3w, r2w, r1w, r0w) * weights0;
	vec4 lw1 = vec4(l7w, l6w, l5w, l4w) * weights1;
	vec4 rw1 = vec4(r7w, r6w, r5w, r4w) * weights1;
	vec4 lw2 = vec4(l11w, l10w, l9w, l8w) * weights2;
	vec4 rw2 = vec4(r11w, r10w, r9w, r8w) * weights2;
	float cw = center_weight;
	float totalw = dot(lw0, vec4(1)) + dot(rw0, vec4(1)) + dot(lw1, vec4(1)) + dot(rw1, vec4(1)) + dot(lw2, vec4(1)) + dot(rw2, vec4(1)) + cw;
	vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
				 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w + 
				 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
				 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w + 
				 l11 * lw2.x + l10 * lw2.y + l9 * lw2.z + l8 * lw2.w +
				 r11 * rw2.x + r10 * rw2.y + r9 * rw2.z + r8 * rw2.w + 
				 c * cw) / totalw;

	gl_FragColor = blur;
}
