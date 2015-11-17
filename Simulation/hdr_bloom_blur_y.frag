
#type frag
#version 450
#extension GL_ARB_bindless_texture : require

out vec4 gl_FragColor;

layout(bindless_sampler) uniform sampler2D unblured_hdr;
layout(bindless_sampler) uniform sampler2D hdr;

const float center_weight = 0.088959;
const vec4 weights0 = vec4(0.060023, 0.071298, 0.080625, 0.086798);
const vec4 weights1 = vec4(0.018438, 0.026663, 0.036706, 0.048107);
const vec4 weights2 = vec4(0.002578, 0.004539, 0.007608, 0.012138);

void main() {
	vec4 l11 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,12), 0);
	vec4 l10 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,11), 0);
	vec4 l9 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,10), 0);
	vec4 l8 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,9), 0);
	vec4 l7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,8), 0);
	vec4 l6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,7), 0);
	vec4 l5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,6), 0);
	vec4 l4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,5), 0);
	vec4 l3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,4), 0);
	vec4 l2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,3), 0);
	vec4 l1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,2), 0);
	vec4 l0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - ivec2(0,1), 0);
	vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	vec4 r0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,1), 0);
	vec4 r1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,2), 0);
	vec4 r2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,3), 0);
	vec4 r3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,4), 0);
	vec4 r4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,5), 0);
	vec4 r5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,6), 0);
	vec4 r6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,7), 0);
	vec4 r7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,8), 0);
	vec4 r8 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,9), 0);
	vec4 r9 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,10), 0);
	vec4 r10 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,11), 0);
	vec4 r11 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + ivec2(0,12), 0);
			
	vec4 lw0 = weights0;
	vec4 rw0 = weights0;
	vec4 lw1 = weights1;
	vec4 rw1 = weights1;
	vec4 lw2 = weights2;
	vec4 rw2 = weights2;
	float cw = center_weight;
	float totalw = dot(lw0, vec4(1)) + dot(rw0, vec4(1)) + dot(lw1, vec4(1)) + dot(rw1, vec4(1)) + dot(lw2, vec4(1)) + dot(rw2, vec4(1)) + cw;
	vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
				 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w + 
				 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
				 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w + 
				 l11 * lw2.x + l10 * lw2.y + l9 * lw2.z + l8 * lw2.w +
				 r11 * rw2.x + r10 * rw2.y + r9 * rw2.z + r8 * rw2.w + 
				 c * cw) / totalw;

	vec4 hdr_texel = texelFetch(unblured_hdr, ivec2(gl_FragCoord.xy), 0);
	vec3 blend = blur.rgb * blur.a + hdr_texel.rgb;
	gl_FragColor = vec4(blend, hdr_texel.w + .1f);
}
