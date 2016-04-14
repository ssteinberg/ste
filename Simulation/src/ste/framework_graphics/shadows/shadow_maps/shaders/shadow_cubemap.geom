
#type geometry
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=36) out;

const int light_buffers_first = 2;
#include "light.glsl"

in vs_out {
	vec2 uv;
	flat int matIdx;
} vin[];

out frag_in {
	vec4 position;
	vec2 uv;
	flat int matIdx;
} vout;

uniform mat4 shadow_transforms[6];

void main() {
	for (int l = 0; l < light_buffer.length(); ++l) {
		vec4 light_pos = vec4(light_buffer[l].position_direction.xyz, 0);

		for (int i = 0; i < 6; ++i) {
			gl_Layer = i + l * 6;

			for (int j = 0; j < 3; ++j) {
				vec4 p = gl_in[j].gl_Position - light_pos;

				vout.uv = vin[j].uv;
				vout.matIdx = vin[j].matIdx;
				vout.position = p;

				gl_Position = shadow_transforms[i] * p;

				EmitVertex();
			}

			EndPrimitive();
		}
	}
}
