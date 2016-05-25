
const int samples = 6;

vec4 hdr_blur(sampler2D hdr, vec2 size, vec2 dir) {
	vec4 one = vec4(1.f);

	vec2 coord = vec2(gl_FragCoord.xy) / size;
	vec2 offset = dir / size;

	vec4 blur = texelFetch(hdr, ivec2(gl_FragCoord.xy), 0) * float(2 << samples);
	for (int i=0; i<samples; ++i) {
		float d = float(i) * 2.f + 1.25f;
		float w = float(1 << (samples - i - 1));

		blur += texture(hdr, coord - offset * d) * w +
				texture(hdr, coord + offset * d) * w;
	}

	float total_w = float((((4 << samples) - 1) ^ (1 << samples)) - 1);
	return blur / total_w;
}
