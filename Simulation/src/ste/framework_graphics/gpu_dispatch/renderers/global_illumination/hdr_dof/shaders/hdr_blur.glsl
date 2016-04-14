
const mediump float center_weight = 0.13298;
const mediump vec4 weights0 = vec4(0.055119, 0.081029, 0.106701, 0.125858);
const mediump vec4 weights1 = vec4(0.003924, 0.008962, 0.018331, 0.033585);

mediump vec4 hdr_blur(sampler2D hdr, ivec2 dir) {
	mediump vec4 l7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 8, 0);
	mediump vec4 l6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 7, 0);
	mediump vec4 l5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 6, 0);
	mediump vec4 l4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 5, 0);
	mediump vec4 l3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 4, 0);
	mediump vec4 l2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 3, 0);
	mediump vec4 l1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 2, 0);
	mediump vec4 l0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 1, 0);
	mediump vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	mediump vec4 r0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 1, 0);
	mediump vec4 r1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 2, 0);
	mediump vec4 r2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 3, 0);
	mediump vec4 r3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 4, 0);
	mediump vec4 r4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 5, 0);
	mediump vec4 r5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 6, 0);
	mediump vec4 r6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 7, 0);
	mediump vec4 r7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 8, 0);

	mediump vec4 lw0 = weights0;
	mediump vec4 rw0 = weights0;
	mediump vec4 lw1 = weights1;
	mediump vec4 rw1 = weights1;
	mediump float cw = center_weight;

	mediump vec4 one = vec4(1.f);
	mediump float totalw = dot(lw0, one) + dot(rw0, one) + dot(lw1, one) + dot(rw1, one) + cw;
	mediump vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
						 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w +
						 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
						 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w +
						 c * cw) / totalw;

	return blur;
}

mediump vec4 bokeh_blur(sampler2D hdr, sampler2D zcoc_buffer, ivec2 dir) {
	mediump vec4 l7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 8, 0);
	mediump vec4 l6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 7, 0);
	mediump vec4 l5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 6, 0);
	mediump vec4 l4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 5, 0);
	mediump vec4 l3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 4, 0);
	mediump vec4 l2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 3, 0);
	mediump vec4 l1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 2, 0);
	mediump vec4 l0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) - dir * 1, 0);
	mediump vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	mediump vec4 r0 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 1, 0);
	mediump vec4 r1 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 2, 0);
	mediump vec4 r2 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 3, 0);
	mediump vec4 r3 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 4, 0);
	mediump vec4 r4 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 5, 0);
	mediump vec4 r5 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 6, 0);
	mediump vec4 r6 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 7, 0);
	mediump vec4 r7 = texelFetch(hdr, ivec2(gl_FragCoord.xy) + dir * 8, 0);

	mediump vec2 z_l7 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 8, 0).rg;
	mediump vec2 z_l6 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 7, 0).rg;
	mediump vec2 z_l5 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 6, 0).rg;
	mediump vec2 z_l4 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 5, 0).rg;
	mediump vec2 z_l3 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 4, 0).rg;
	mediump vec2 z_l2 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 3, 0).rg;
	mediump vec2 z_l1 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 2, 0).rg;
	mediump vec2 z_l0 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) - dir * 1, 0).rg;
	mediump float z_c = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy), 0).r;
	mediump vec2 z_r0 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 1, 0).rg;
	mediump vec2 z_r1 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 2, 0).rg;
	mediump vec2 z_r2 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 3, 0).rg;
	mediump vec2 z_r3 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 4, 0).rg;
	mediump vec2 z_r4 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 5, 0).rg;
	mediump vec2 z_r5 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 6, 0).rg;
	mediump vec2 z_r6 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 7, 0).rg;
	mediump vec2 z_r7 = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy) + dir * 8, 0).rg;

	mediump float l7w = clamp(mix(1, 0, (z_l7.x - z_c) / .04f), 0, 1) * l7.w * z_l7.y;
	mediump float l6w = clamp(mix(1, 0, (z_l6.x - z_c) / .04f), 0, 1) * l6.w * z_l6.y;
	mediump float l5w = clamp(mix(1, 0, (z_l5.x - z_c) / .04f), 0, 1) * l5.w * z_l5.y;
	mediump float l4w = clamp(mix(1, 0, (z_l4.x - z_c) / .04f), 0, 1) * l4.w * z_l4.y;
	mediump float l3w = clamp(mix(1, 0, (z_l3.x - z_c) / .04f), 0, 1) * l3.w * z_l3.y;
	mediump float l2w = clamp(mix(1, 0, (z_l2.x - z_c) / .04f), 0, 1) * l2.w * z_l2.y;
	mediump float l1w = clamp(mix(1, 0, (z_l1.x - z_c) / .04f), 0, 1) * l1.w * z_l1.y;
	mediump float l0w = clamp(mix(1, 0, (z_l0.x - z_c) / .04f), 0, 1) * l0.w * z_l0.y;
	mediump float r0w = clamp(mix(1, 0, (z_r0.x - z_c) / .04f), 0, 1) * r0.w * z_r0.y;
	mediump float r1w = clamp(mix(1, 0, (z_r1.x - z_c) / .04f), 0, 1) * r1.w * z_r1.y;
	mediump float r2w = clamp(mix(1, 0, (z_r2.x - z_c) / .04f), 0, 1) * r2.w * z_r2.y;
	mediump float r3w = clamp(mix(1, 0, (z_r3.x - z_c) / .04f), 0, 1) * r3.w * z_r3.y;
	mediump float r4w = clamp(mix(1, 0, (z_r4.x - z_c) / .04f), 0, 1) * r4.w * z_r4.y;
	mediump float r5w = clamp(mix(1, 0, (z_r5.x - z_c) / .04f), 0, 1) * r5.w * z_r5.y;
	mediump float r6w = clamp(mix(1, 0, (z_r6.x - z_c) / .04f), 0, 1) * r6.w * z_r6.y;
	mediump float r7w = clamp(mix(1, 0, (z_r7.x - z_c) / .04f), 0, 1) * r7.w * z_r7.y;

	mediump vec4 lw0 = vec4(l3w, l2w, l1w, l0w) * weights0;
	mediump vec4 rw0 = vec4(r3w, r2w, r1w, r0w) * weights0;
	mediump vec4 lw1 = vec4(l7w, l6w, l5w, l4w) * weights1;
	mediump vec4 rw1 = vec4(r7w, r6w, r5w, r4w) * weights1;
	mediump float cw = center_weight * mix(.1f, 1.f, c.w + .1f);

	mediump vec4 one = vec4(1.f);
	mediump float totalw = dot(lw0, one) + dot(rw0, one) + dot(lw1, one) + dot(rw1, one) + cw;
	mediump vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
						 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w +
				 		 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
				 		 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w +
						 c * cw) / totalw;

	return blur;
}
