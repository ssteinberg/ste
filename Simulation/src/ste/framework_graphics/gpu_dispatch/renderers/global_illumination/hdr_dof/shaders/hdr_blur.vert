
#type vert
#version 450

layout(location = 0) in vec3 vert;
layout(location = 3) in vec2 tc;

const int blur_samples_count = 8;

out vec4 gl_Position;
out vs_out {
	vec2 uv;
	vec2 blur_uvs[blur_samples_count];
} vout;

uniform vec2 size;
uniform vec2 dir;

void main() {
	vout.uv = tc;

	vec2 offset = dir / size;
	for (int i = 0; i < blur_samples_count; ++i) {
		int after_middle = i >= blur_samples_count / 2 ? 1 : 0;
		vout.blur_uvs[i] = tc + offset * (float(i - blur_samples_count / 2 + after_middle) * 2.f - .5f);
	}

	gl_Position = vec4(vert, 1);
}
