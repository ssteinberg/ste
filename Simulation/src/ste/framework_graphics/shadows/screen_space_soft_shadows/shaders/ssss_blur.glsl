
const mediump float center_weight = 0.13298;
const mediump vec4 weights0 = vec4(0.055119, 0.081029, 0.106701, 0.125858);
const mediump vec4 weights1 = vec4(0.003924, 0.008962, 0.018331, 0.033585);

mediump vec4 ssss_blur(sampler2DArray src, int layer, ivec2 dir) {
	mediump vec4 l7 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 8);
	mediump vec4 l6 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 7);
	mediump vec4 l5 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 6);
	mediump vec4 l4 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 5);
	mediump vec4 l3 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 4);
	mediump vec4 l2 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 3);
	mediump vec4 l1 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 2);
	mediump vec4 l0 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, -dir * 1);
	mediump vec4 c  = texelFetch(src, ivec3(gl_FragCoord.xy, layer), 0);
	mediump vec4 r0 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 1);
	mediump vec4 r1 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 2);
	mediump vec4 r2 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 3);
	mediump vec4 r3 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 4);
	mediump vec4 r4 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 5);
	mediump vec4 r5 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 6);
	mediump vec4 r6 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 7);
	mediump vec4 r7 = texelFetchOffset(src, ivec3(gl_FragCoord.xy, layer), 0, +dir * 8);

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
