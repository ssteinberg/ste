
#type geometry
#version 440

layout(triangles) in;
layout(triangle_strip, max_vertices=90) out;

const int light_buffers_first = 2;

#include "light.glsl"

uniform mat4 shadow_transforms[6];

out vec4 position;

void main() {
	for (int l = 0; l < light_buffer.length(); ++l) {
		vec4 light_pos = vec4(light_buffer[l].position_direction.xyz, 0);

		for(int i = 0; i < 6; ++i) {
			gl_Layer = i + l*6;

			position = gl_in[0].gl_Position - light_pos;
			gl_Position = shadow_transforms[i] * position;
			EmitVertex();

			position = gl_in[1].gl_Position - light_pos;
			gl_Position = shadow_transforms[i] * position;
			EmitVertex();

			position = gl_in[2].gl_Position - light_pos;
			gl_Position = shadow_transforms[i] * position;
			EmitVertex();

			EndPrimitive();
		}
	}
}
