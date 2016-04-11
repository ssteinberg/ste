
#type geometry
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=36) out;

const int light_buffers_first = 2;
#include "light.glsl"

uniform mat4 shadow_transforms[6];

in vs_out {
	vec2 uv;
	flat int matIdx;
} vin[];

out geo_out {
	vec2 uv;
	flat int matIdx;
} vout;

out vec4 position;

void main() {
	for (int l = 0; l < light_buffer.length(); ++l) {
		vec4 light_pos = vec4(light_buffer[l].position_direction.xyz, 0);

		for(int i = 0; i < 6; ++i) {
			gl_Layer = i + l*6;

			for (int j = 0; j < 3; ++j) {
				vout.uv = vin[j].uv;
				vout.matIdx = vin[j].matIdx;

				position = gl_in[j].gl_Position - light_pos;
				gl_Position = shadow_transforms[i] * position;

				EmitVertex();
			}

			EndPrimitive();
		}
	}
}
