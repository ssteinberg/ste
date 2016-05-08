
const float center_weight = 0.181231;
const vec4 weights = vec4(0.040897, 0.078445, 0.124913, 0.16513);

vec4 hdr_blur(sampler2D hdr, vec2 uv, vec2 uvs[blur_samples_count]) {
	vec4 one = vec4(1.f);

	vec4 l3 = textureLod(hdr, uvs[0], 0);
	vec4 l2 = textureLod(hdr, uvs[1], 0);
	vec4 l1 = textureLod(hdr, uvs[2], 0);
	vec4 l0 = textureLod(hdr, uvs[3], 0);
	vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	vec4 r0 = textureLod(hdr, uvs[4], 0);
	vec4 r1 = textureLod(hdr, uvs[5], 0);
	vec4 r2 = textureLod(hdr, uvs[6], 0);
	vec4 r3 = textureLod(hdr, uvs[7], 0);

	vec4 lw = weights;
	vec4 rw = weights;
	float cw = center_weight;

	vec4 blur = l3 * lw.x + l2 * lw.y + l1 * lw.z + l0 * lw.w +
				r3 * rw.x + r2 * rw.y + r1 * rw.z + r0 * rw.w +
				c * cw;

	return blur;
}

vec4 bokeh_blur(sampler2D hdr, sampler2D zcoc_buffer, vec2 uv, vec2 uvs[blur_samples_count]) {
	vec4 one = vec4(1.f);
	vec4 zero = vec4(.0f);

	vec4 l3 = textureLod(hdr, uvs[0], 0);
	vec4 l2 = textureLod(hdr, uvs[1], 0);
	vec4 l1 = textureLod(hdr, uvs[2], 0);
	vec4 l0 = textureLod(hdr, uvs[3], 0);
	vec4 c  = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0);
	vec4 r0 = textureLod(hdr, uvs[4], 0);
	vec4 r1 = textureLod(hdr, uvs[5], 0);
	vec4 r2 = textureLod(hdr, uvs[6], 0);
	vec4 r3 = textureLod(hdr, uvs[7], 0);

	vec2 z_l3 = textureLod(zcoc_buffer, uvs[0], 0).xy;
	vec2 z_l2 = textureLod(zcoc_buffer, uvs[1], 0).xy;
	vec2 z_l1 = textureLod(zcoc_buffer, uvs[2], 0).xy;
	vec2 z_l0 = textureLod(zcoc_buffer, uvs[3], 0).xy;
	float z_c = texelFetch(zcoc_buffer, ivec2(gl_FragCoord.xy), 0).x;
	vec2 z_r0 = textureLod(zcoc_buffer, uvs[4], 0).xy;
	vec2 z_r1 = textureLod(zcoc_buffer, uvs[5], 0).xy;
	vec2 z_r2 = textureLod(zcoc_buffer, uvs[6], 0).xy;
	vec2 z_r3 = textureLod(zcoc_buffer, uvs[7], 0).xy;

	vec4 l = vec4(l3.w, l2.w, l1.w, l0.w);
	vec4 r = vec4(r3.w, r2.w, r1.w, r0.w);

	vec4 z_l_x = vec4(z_l3.x, z_l2.x, z_l1.x, z_l0.x);
	vec4 z_l_y = vec4(z_l3.y, z_l2.y, z_l1.y, z_l0.y);
	vec4 z_r_x = vec4(z_r3.x, z_r2.x, z_r1.x, z_r0.x);
	vec4 z_r_y = vec4(z_r3.y, z_r2.y, z_r1.y, z_r0.y);

	vec4 l_w = clamp(mix(one, zero, (z_l_x - z_c.xxxx) / .04f), .0f, 1.f) * l * z_l_y;
	vec4 r_w = clamp(mix(one, zero, (z_r_x - z_c.xxxx) / .04f), .0f, 1.f) * r * z_r_y;

	vec4 lw = l_w * weights;
	vec4 rw = r_w * weights;
	float cw = center_weight * mix(.1f, 1.f, c.w + .1f);

	float totalw = dot(lw, one) + dot(rw, one) + cw;
	vec4 blur = (l3 * lw.x + l2 * lw.y + l1 * lw.z + l0 * lw.w +
				 r3 * rw.x + r2 * rw.y + r1 * rw.z + r0 * rw.w +
				 c * cw) / totalw;

	return blur;
}
