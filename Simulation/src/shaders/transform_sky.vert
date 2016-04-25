
#type vert
#version 450

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 tex_coords;

out vec4 gl_Position;
out vec3 frag_position;
out vec3 frag_wposition;
out vec2 frag_texcoords;

uniform mat4 projection;
uniform mat4 view_model;
uniform float near = 5.0f;
uniform float far = 10000.0f;

void main() {
	vec3 p = vert * far * .99f + vec3(0, -far * .05f, 0);
	vec4 eye_v = view_model * vec4(p, 1);

	frag_position = eye_v.xyz;
	frag_wposition = p;
	frag_texcoords = tex_coords;

	gl_Position = projection * eye_v;
}
