
#type geometry
#version 440

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

uniform mat4 shadow_transforms[6];

out vec4 position;

void main() {
	for(int i = 0; i < 6; ++i) {
		gl_Layer = i;

		position = gl_in[0].gl_Position;
		gl_Position = shadow_transforms[i] * position;
		EmitVertex();

		position = gl_in[1].gl_Position;
		gl_Position = shadow_transforms[i] * position;
		EmitVertex();

		position = gl_in[2].gl_Position;
		gl_Position = shadow_transforms[i] * position;
		EmitVertex();

		EndPrimitive();
	}
}
