
#type frag
#version 440

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D hdr;
layout(binding = 1) uniform sampler2D coc;

const float center_weight = 0.08745;
const vec4 weights1 = vec4(0.024418,	0.032928,	0.042669,	0.05313);
const vec4 weights0 = vec4(0.06357,	0.073088,	0.080748,	0.085724);

void main() {
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
	
	vec2 c_l7 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(8,0), 0).rg;
	vec2 c_l6 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(7,0), 0).rg;
	vec2 c_l5 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(6,0), 0).rg;
	vec2 c_l4 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(5,0), 0).rg;
	vec2 c_l3 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(4,0), 0).rg;
	vec2 c_l2 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(3,0), 0).rg;
	vec2 c_l1 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(2,0), 0).rg;
	vec2 c_l0 = texelFetch(coc, ivec2(gl_FragCoord.xy) - ivec2(1,0), 0).rg;
	vec2 c_c  = texelFetch(coc, ivec2(gl_FragCoord.xy), 0).rg;
	vec2 c_r0 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(1,0), 0).rg;
	vec2 c_r1 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(2,0), 0).rg;
	vec2 c_r2 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(3,0), 0).rg;
	vec2 c_r3 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(4,0), 0).rg;
	vec2 c_r4 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(5,0), 0).rg;
	vec2 c_r5 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(6,0), 0).rg;
	vec2 c_r6 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(7,0), 0).rg;
	vec2 c_r7 = texelFetch(coc, ivec2(gl_FragCoord.xy) + ivec2(8,0), 0).rg;
	
	float l7w = clamp(mix(1, 0, (c_l7.y - c_c.y) / .04f), 0, 1) * l7.w * c_l7.x;
	float l6w = clamp(mix(1, 0, (c_l6.y - c_c.y) / .04f), 0, 1) * l6.w * c_l6.x;
	float l5w = clamp(mix(1, 0, (c_l5.y - c_c.y) / .04f), 0, 1) * l5.w * c_l5.x;
	float l4w = clamp(mix(1, 0, (c_l4.y - c_c.y) / .04f), 0, 1) * l4.w * c_l4.x;
	float l3w = clamp(mix(1, 0, (c_l3.y - c_c.y) / .04f), 0, 1) * l3.w * c_l3.x;
	float l2w = clamp(mix(1, 0, (c_l2.y - c_c.y) / .04f), 0, 1) * l2.w * c_l2.x;
	float l1w = clamp(mix(1, 0, (c_l1.y - c_c.y) / .04f), 0, 1) * l1.w * c_l1.x;
	float l0w = clamp(mix(1, 0, (c_l0.y - c_c.y) / .04f), 0, 1) * l0.w * c_l0.x;
	float r0w = clamp(mix(1, 0, (c_r0.y - c_c.y) / .04f), 0, 1) * r0.w * c_r0.x;
	float r1w = clamp(mix(1, 0, (c_r1.y - c_c.y) / .04f), 0, 1) * r1.w * c_r1.x;
	float r2w = clamp(mix(1, 0, (c_r2.y - c_c.y) / .04f), 0, 1) * r2.w * c_r2.x;
	float r3w = clamp(mix(1, 0, (c_r3.y - c_c.y) / .04f), 0, 1) * r3.w * c_r3.x;
	float r4w = clamp(mix(1, 0, (c_r4.y - c_c.y) / .04f), 0, 1) * r4.w * c_r4.x;
	float r5w = clamp(mix(1, 0, (c_r5.y - c_c.y) / .04f), 0, 1) * r5.w * c_r5.x;
	float r6w = clamp(mix(1, 0, (c_r6.y - c_c.y) / .04f), 0, 1) * r6.w * c_r6.x;
	float r7w = clamp(mix(1, 0, (c_r7.y - c_c.y) / .04f), 0, 1) * r7.w * c_r7.x;
			
	vec4 lw0 = vec4(l3w, l2w, l1w, l0w) * weights0;
	vec4 rw0 = vec4(r3w, r2w, r1w, r0w) * weights0;
	vec4 lw1 = vec4(l7w, l6w, l5w, l4w) * weights1;
	vec4 rw1 = vec4(r7w, r6w, r5w, r4w) * weights1;
	float totalw = dot(lw0, vec4(1)) + dot(rw0, vec4(1)) + dot(lw1, vec4(1)) + dot(rw1, vec4(1)) + center_weight;
	vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
				 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w + 
				 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
				 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w + 
				 c * center_weight) / totalw;

	gl_FragColor = blur;
}
