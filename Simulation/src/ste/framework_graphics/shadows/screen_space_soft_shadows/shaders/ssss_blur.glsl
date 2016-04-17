
mediump vec2 ssss_blur_decompose(sampler2DArray src, sampler2D z_buffer, int layer, const ivec2 dir) {
	const ivec2 offsets_r_1_4[4] = {
		dir * 1,
		dir * 2,
		dir * 3,
		dir * 4
	};
	const ivec2 offsets_r_5_8[4] = {
		dir * 5,
		dir * 6,
		dir * 7,
		dir * 8
	};
	const ivec2 offsets_r_9_12[4] = {
		dir * 9,
		dir * 10,
		dir * 11,
		dir * 12
	};
	const ivec2 offsets_l_1_4[4] = {
		-dir * 1,
		-dir * 2,
		-dir * 3,
		-dir * 4
	};
	const ivec2 offsets_l_5_8[4] = {
		-dir * 5,
		-dir * 6,
		-dir * 7,
		-dir * 8
	};
	const ivec2 offsets_l_9_12[4] = {
		-dir * 9,
		-dir * 10,
		-dir * 11,
		-dir * 12
	};
	mediump vec4 zero = vec4(.0f);
	mediump vec4 one = vec4(1.f);
	mediump vec4 c1_4 = vec4(.5f, 1.5f, 2.5f, 3.5f);
	mediump vec4 c5_8 = vec4(4.5f, 5.5f, 6.5f, 7.5f);
	mediump vec4 c9_12 = vec4(8.5f, 9.5f, 10.5f, 11.5f);
	mediump float z_cutoff = .02f;

	mediump float penumbra = texelFetch(src, ivec3(gl_FragCoord.xy, layer), 0).x;
	mediump vec4 penumbra_r1_4 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[3]).x );
	mediump vec4 penumbra_l1_4 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[3]).x );
	mediump vec4 penumbra_r5_8 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[3]).x );
	mediump vec4 penumbra_l5_8 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[3]).x );
	mediump vec4 penumbra_r9_12 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[3]).x );
	mediump vec4 penumbra_l9_12 = vec4(	texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[0]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[1]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[2]).x,
										texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[3]).x );

	mediump float depth = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	mediump vec4 d4 = vec4(depth);
	mediump vec4 depth_r1_4 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[3]).x );
	mediump vec4 depth_l1_4 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[3]).x );
	mediump vec4 depth_r5_8 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[3]).x );
	mediump vec4 depth_l5_8 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[3]).x );
	mediump vec4 depth_r9_12 = vec4(texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[3]).x );
	mediump vec4 depth_l9_12 = vec4(texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[3]).x );

	mediump vec4 shadow_r1_4 = sign(penumbra_r1_4);
	mediump vec4 shadow_r5_8 = sign(penumbra_r5_8);
	mediump vec4 shadow_r9_12 = sign(penumbra_r9_12);
	mediump vec4 shadow_l1_4 = sign(penumbra_l1_4);
	mediump vec4 shadow_l5_8 = sign(penumbra_l5_8);
	mediump vec4 shadow_l9_12 = sign(penumbra_l9_12);
	mediump float shadow = sign(penumbra);

	mediump vec4 pw_l1_4 = max(one - c1_4 / penumbra_l1_4, zero);
	mediump vec4 pw_r1_4 = max(one - c1_4 / penumbra_r1_4, zero);
	mediump vec4 pw_l5_8 = max(one - c5_8 / penumbra_l5_8, zero);
	mediump vec4 pw_r5_8 = max(one - c5_8 / penumbra_r5_8, zero);
	mediump vec4 pw_l9_12 = max(one - c9_12 / penumbra_l9_12, zero);
	mediump vec4 pw_r9_12 = max(one - c9_12 / penumbra_r9_12, zero);

	mediump vec4 z_dif_r1_4 = one - min(one, abs(d4 - depth_r1_4) / z_cutoff);
	mediump vec4 z_dif_l1_4 = one - min(one, abs(d4 - depth_l1_4) / z_cutoff);
	mediump vec4 z_dif_r5_8 = one - min(one, abs(d4 - depth_r5_8) / z_cutoff);
	mediump vec4 z_dif_l5_8 = one - min(one, abs(d4 - depth_l5_8) / z_cutoff);
	mediump vec4 z_dif_r9_12 = one - min(one, abs(d4 - depth_r9_12) / z_cutoff);
	mediump vec4 z_dif_l9_12 = one - min(one, abs(d4 - depth_l9_12) / z_cutoff);

	mediump vec4 lw0 = pw_l1_4 * z_dif_l1_4;
	mediump vec4 rw0 = pw_r1_4 * z_dif_r1_4;
	mediump vec4 lw1 = pw_l5_8 * z_dif_l5_8;
	mediump vec4 rw1 = pw_r5_8 * z_dif_r5_8;
	mediump vec4 lw2 = pw_l9_12 * z_dif_l9_12;
	mediump vec4 rw2 = pw_r9_12 * z_dif_r9_12;
	mediump float cw = 1.f;

	mediump float totalw = dot(lw0, one) + dot(rw0, one) +
						   dot(lw1, one) + dot(rw1, one) +
						   dot(lw2, one) + dot(rw2, one) +
						   cw;

	mediump vec4 vpen = penumbra_l1_4 * lw0 + penumbra_r1_4 * rw0 +
						penumbra_l5_8 * lw1 + penumbra_r5_8 * rw1 +
						penumbra_l9_12 * lw2 + penumbra_r9_12 * rw2;
	mediump float pen = (dot(vpen, one) + penumbra * cw) / totalw;

	mediump vec4 vshd = shadow_l1_4 * lw0 + shadow_r1_4 * rw0 +
						shadow_l5_8 * lw1 + shadow_r5_8 * rw1 +
						shadow_l9_12 * lw2 + shadow_r9_12 * rw2;
	mediump float shd = (dot(vshd, one) + shadow * cw) / totalw;

	return vec2(shd, pen);
}

mediump float ssss_blur(sampler2DArray src, sampler2D z_buffer, int layer, const ivec2 dir) {
	const ivec2 offsets_r_1_4[4] = {
		dir * 1,
		dir * 2,
		dir * 3,
		dir * 4
	};
	const ivec2 offsets_r_5_8[4] = {
		dir * 5,
		dir * 6,
		dir * 7,
		dir * 8
	};
	const ivec2 offsets_r_9_12[4] = {
		dir * 9,
		dir * 10,
		dir * 11,
		dir * 12
	};
	const ivec2 offsets_l_1_4[4] = {
		-dir * 1,
		-dir * 2,
		-dir * 3,
		-dir * 4
	};
	const ivec2 offsets_l_5_8[4] = {
		-dir * 5,
		-dir * 6,
		-dir * 7,
		-dir * 8
	};
	const ivec2 offsets_l_9_12[4] = {
		-dir * 9,
		-dir * 10,
		-dir * 11,
		-dir * 12
	};
	mediump vec4 zero = vec4(.0f);
	mediump vec4 one = vec4(1.f);
	mediump vec4 c1_4 = vec4(.5f, 1.5f, 2.5f, 3.5f);
	mediump vec4 c5_8 = vec4(4.5f, 5.5f, 6.5f, 7.5f);
	mediump vec4 c9_12 = vec4(8.5f, 9.5f, 10.5f, 11.5f);
	mediump float z_cutoff = .02f;

	mediump vec2 c = texelFetch(src, ivec3(gl_FragCoord.xy, layer), 0).xy;
	mediump float shadow = c.x;
	mediump float penumbra = c.y;

	mediump vec2 texels_r1 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[0]).xy;
	mediump vec2 texels_r2 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[1]).xy;
	mediump vec2 texels_r3 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[2]).xy;
	mediump vec2 texels_r4 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_1_4[3]).xy;
	mediump vec2 texels_l1 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[0]).xy;
	mediump vec2 texels_l2 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[1]).xy;
	mediump vec2 texels_l3 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[2]).xy;
	mediump vec2 texels_l4 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_1_4[3]).xy;
	mediump vec2 texels_r5 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[0]).xy;
	mediump vec2 texels_r6 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[1]).xy;
	mediump vec2 texels_r7 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[2]).xy;
	mediump vec2 texels_r8 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_5_8[3]).xy;
	mediump vec2 texels_l5 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[0]).xy;
	mediump vec2 texels_l6 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[1]).xy;
	mediump vec2 texels_l7 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[2]).xy;
	mediump vec2 texels_l8 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_5_8[3]).xy;
	mediump vec2 texels_r9 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[0]).xy;
	mediump vec2 texels_r10 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[1]).xy;
	mediump vec2 texels_r11 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[2]).xy;
	mediump vec2 texels_r12 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_r_9_12[3]).xy;
	mediump vec2 texels_l9 =  texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[0]).xy;
	mediump vec2 texels_l10 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[1]).xy;
	mediump vec2 texels_l11 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[2]).xy;
	mediump vec2 texels_l12 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, offsets_l_9_12[3]).xy;

	mediump float depth = texelFetch(z_buffer, ivec2(gl_FragCoord.xy), 0).x;
	mediump vec4 d4 = vec4(depth);
	mediump vec4 depth_r1_4 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_1_4[3]).x );
	mediump vec4 depth_l1_4 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_1_4[3]).x );
	mediump vec4 depth_r5_8 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_5_8[3]).x );
	mediump vec4 depth_l5_8 = vec4(	texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_5_8[3]).x );
	mediump vec4 depth_r9_12 = vec4(texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_r_9_12[3]).x );
	mediump vec4 depth_l9_12 = vec4(texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[0]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[1]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[2]).x,
									texelFetchOffset(z_buffer, ivec2(gl_FragCoord.xy), 0, offsets_l_9_12[3]).x );

	mediump vec4 penumbra_r1_4 = vec4(texels_r1.y, texels_r2.y, texels_r3.y, texels_r4.y);
	mediump vec4 shadow_r1_4 =   vec4(texels_r1.x, texels_r2.x, texels_r3.x, texels_r4.x);

	mediump vec4 penumbra_l1_4 = vec4(texels_l1.y, texels_l2.y, texels_l3.y, texels_l4.y);
	mediump vec4 shadow_l1_4 =   vec4(texels_l1.x, texels_l2.x, texels_l3.x, texels_l4.x);

	mediump vec4 penumbra_r5_8 = vec4(texels_r5.y, texels_r6.y, texels_r7.y, texels_r8.y);
	mediump vec4 shadow_r5_8 =   vec4(texels_r5.x, texels_r6.x, texels_r7.x, texels_r8.x);

	mediump vec4 penumbra_l5_8 = vec4(texels_l5.y, texels_l6.y, texels_l7.y, texels_l8.y);
	mediump vec4 shadow_l5_8 =   vec4(texels_l5.x, texels_l6.x, texels_l7.x, texels_l8.x);

	mediump vec4 penumbra_r9_12 = vec4(texels_r9.y, texels_r10.y, texels_r11.y, texels_r12.y);
	mediump vec4 shadow_r9_12 =   vec4(texels_r9.x, texels_r10.x, texels_r11.x, texels_r12.x);

	mediump vec4 penumbra_l9_12 = vec4(texels_l9.y, texels_l10.y, texels_l11.y, texels_l12.y);
	mediump vec4 shadow_l9_12 =   vec4(texels_l9.x, texels_l10.x, texels_l11.x, texels_l12.x);

	mediump vec4 pw_l1_4 = max(one - c1_4 / penumbra_l1_4, zero);
	mediump vec4 pw_r1_4 = max(one - c1_4 / penumbra_r1_4, zero);
	mediump vec4 pw_l5_8 = max(one - c5_8 / penumbra_l5_8, zero);
	mediump vec4 pw_r5_8 = max(one - c5_8 / penumbra_r5_8, zero);
	mediump vec4 pw_l9_12 = max(one - c9_12 / penumbra_l9_12, zero);
	mediump vec4 pw_r9_12 = max(one - c9_12 / penumbra_r9_12, zero);

	mediump vec4 z_dif_r1_4 = one - min(one, abs(d4 - depth_r1_4) / z_cutoff);
	mediump vec4 z_dif_l1_4 = one - min(one, abs(d4 - depth_l1_4) / z_cutoff);
	mediump vec4 z_dif_r5_8 = one - min(one, abs(d4 - depth_r5_8) / z_cutoff);
	mediump vec4 z_dif_l5_8 = one - min(one, abs(d4 - depth_l5_8) / z_cutoff);
	mediump vec4 z_dif_r9_12 = one - min(one, abs(d4 - depth_r9_12) / z_cutoff);
	mediump vec4 z_dif_l9_12 = one - min(one, abs(d4 - depth_l9_12) / z_cutoff);

	mediump vec4 lw0 = pw_l1_4 * z_dif_l1_4;
	mediump vec4 rw0 = pw_r1_4 * z_dif_r1_4;
	mediump vec4 lw1 = pw_l5_8 * z_dif_l5_8;
	mediump vec4 rw1 = pw_r5_8 * z_dif_r5_8;
	mediump vec4 lw2 = pw_l9_12 * z_dif_l9_12;
	mediump vec4 rw2 = pw_r9_12 * z_dif_r9_12;
	mediump float cw = 1.f;

	mediump float totalw = dot(lw0, one) + dot(rw0, one) +
						   dot(lw1, one) + dot(rw1, one) +
						   dot(lw2, one) + dot(rw2, one) +
						   cw;

	mediump vec4 vshd = shadow_l1_4 * lw0 + shadow_r1_4 * rw0 +
						shadow_l5_8 * lw1 + shadow_r5_8 * rw1 +
						shadow_l9_12 * lw2 + shadow_r9_12 * rw2;
	mediump float shd = (dot(vshd, one) + shadow * cw) / totalw;

	return shd;
}
