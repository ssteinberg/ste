
const mediump float center_weight = 0.13298;
const mediump vec4 weights0 = vec4(0.055119, 0.081029, 0.106701, 0.125858);
const mediump vec4 weights1 = vec4(0.003924, 0.008962, 0.018331, 0.033585);

mediump vec4 hdr_blur(sampler2D hdr, ivec2 dir) {
	mediump vec4 l7 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 8);
	mediump vec4 l6 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 7);
	mediump vec4 l5 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 6);
	mediump vec4 l4 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 5);
	mediump vec4 l3 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 4);
	mediump vec4 l2 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 3);
	mediump vec4 l1 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 2);
	mediump vec4 l0 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 1);
	mediump vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	mediump vec4 r0 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 1);
	mediump vec4 r1 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 2);
	mediump vec4 r2 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 3);
	mediump vec4 r3 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 4);
	mediump vec4 r4 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 5);
	mediump vec4 r5 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 6);
	mediump vec4 r6 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 7);
	mediump vec4 r7 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 8);

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
	vec4 one = vec4(1.f);
	vec4 zero = vec4(.0f);

	mediump vec4 l7 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 8);
	mediump vec4 l6 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 7);
	mediump vec4 l5 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 6);
	mediump vec4 l4 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 5);
	mediump vec4 l3 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 4);
	mediump vec4 l2 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 3);
	mediump vec4 l1 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 2);
	mediump vec4 l0 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, -dir * 1);
	mediump vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	mediump vec4 r0 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 1);
	mediump vec4 r1 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 2);
	mediump vec4 r2 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 3);
	mediump vec4 r3 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 4);
	mediump vec4 r4 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 5);
	mediump vec4 r5 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 6);
	mediump vec4 r6 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 7);
	mediump vec4 r7 = texelFetchOffset(hdr, ivec2(gl_FragCoord.xy), 0, +dir * 8);

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

	mediump vec4 l_74 = vec4(l7.w, l6.w, l5.w, l4.w);
	mediump vec4 l_30 = vec4(l3.w, l2.w, l1.w, l0.w);
	mediump vec4 r_74 = vec4(r7.w, r6.w, r5.w, r4.w);
	mediump vec4 r_30 = vec4(r3.w, r2.w, r1.w, r0.w);

	mediump vec4 z_l_30_x = vec4(z_l3.x, z_l2.x, z_l1.x, z_l0.x);
	mediump vec4 z_l_74_x = vec4(z_l7.x, z_l6.x, z_l5.x, z_l4.x);
	mediump vec4 z_l_30_y = vec4(z_l3.y, z_l2.y, z_l1.y, z_l0.y);
	mediump vec4 z_l_74_y = vec4(z_l7.y, z_l6.y, z_l5.y, z_l4.y);
	mediump vec4 z_r_30_x = vec4(z_r3.x, z_r2.x, z_r1.x, z_r0.x);
	mediump vec4 z_r_74_x = vec4(z_r7.x, z_r6.x, z_r5.x, z_r4.x);
	mediump vec4 z_r_30_y = vec4(z_r3.y, z_r2.y, z_r1.y, z_r0.y);
	mediump vec4 z_r_74_y = vec4(z_r7.y, z_r6.y, z_r5.y, z_r4.y);

	mediump vec4 l_74_w = clamp(mix(one, zero, (z_l_74_x - z_c.xxxx) / .04f), 0, 1) * l_74 * z_l_74_y;
	mediump vec4 l_30_w = clamp(mix(one, zero, (z_l_30_x - z_c.xxxx) / .04f), 0, 1) * l_30 * z_l_30_y;
	mediump vec4 r_74_w = clamp(mix(one, zero, (z_r_74_x - z_c.xxxx) / .04f), 0, 1) * r_74 * z_r_74_y;
	mediump vec4 r_30_w = clamp(mix(one, zero, (z_r_30_x - z_c.xxxx) / .04f), 0, 1) * r_30 * z_r_30_y;

	mediump vec4 lw0 = l_30_w * weights0;
	mediump vec4 rw0 = r_30_w * weights0;
	mediump vec4 lw1 = l_74_w * weights1;
	mediump vec4 rw1 = r_74_w * weights1;
	mediump float cw = center_weight * mix(.1f, 1.f, c.w + .1f);

	mediump float totalw = dot(lw0, one) + dot(rw0, one) + dot(lw1, one) + dot(rw1, one) + cw;
	mediump vec4 blur = (l3 * lw0.x + l2 * lw0.y + l1 * lw0.z + l0 * lw0.w +
						 r3 * rw0.x + r2 * rw0.y + r1 * rw0.z + r0 * rw0.w +
				 		 l7 * lw1.x + l6 * lw1.y + l5 * lw1.z + l4 * lw1.w +
				 		 r7 * rw1.x + r6 * rw1.y + r5 * rw1.z + r4 * rw1.w +
						 c * cw) / totalw;

	return blur;
}
