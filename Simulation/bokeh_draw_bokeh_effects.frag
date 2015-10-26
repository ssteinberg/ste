
#type frag
#version 450

out vec4 gl_FragColor;

layout(binding = 3) uniform sampler2D tex;

in fp_in {
	vec4 color;
	vec2 uv;
} vin;

void main() {
	float alpha_mask = texture(tex, vin.uv).x;
	if (alpha_mask < .01f)
		discard;
	gl_FragColor = vec4(vin.color.rgb, alpha_mask);
}
