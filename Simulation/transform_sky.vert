
#type vert
#version 450

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

out vec4 gl_Position;
out vec3 frag_position;
out vec2 frag_texcoords;

uniform mat4 projection;
uniform mat4 view_model;
uniform float far = 10000.0f;

void main() {
	vec4 eye_v = view_model * vec4(vert * (far * .99f) + vec3(0, -far * .05f, 0), 1);

	frag_position = eye_v.xyz;
	frag_texcoords = tex_coords;

	gl_Position = projection * eye_v;
}
