
const int samples = 6;

vec4 hdr_blur(sampler2D hdr, vec2 size, vec2 dir) {
	vec4 one = vec4(1.f);

	vec2 coord = vec2(gl_FragCoord.xy) / size;
	vec2 offset = dir / size;

	float kernel[samples + 1] = { 0.002406f, 0.009255f, 0.027867f, 0.065666f, 0.121117f, 0.174868f, 0.197641f };

	float total_w = kernel[samples];
	vec4 blur = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0) * kernel[samples];
	for (int i=0; i<samples; ++i) {
		float d = float(i) * 2.f + 1.5f;
		float w = kernel[samples - i - 1];

		total_w += 2.f * w;
		blur += (texture(hdr, coord - offset * d) +
				 texture(hdr, coord + offset * d)) * w;
	}

	return blur / total_w;
}
