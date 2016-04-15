
#type geometry
#version 450

layout(triangles, invocations=2) in;
layout(triangle_strip, max_vertices=15) out;

const int light_buffers_first = 2;
#include "light.glsl"

in vs_out {
	vec3 normal;
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
	int l = gl_InvocationID;
	vec4 light_pos = vec4(light_buffer[l].position_direction.xyz, 0);

	for (int i = 0; i < 6; ++i) {
		vec3 N = (vin[0].normal + vin[1].normal + vin[2].normal) / 3.f;
		vec3 V = light_pos.xyz - gl_in[0].gl_Position.xyz;

		if (dot(N ,V) > 0) {
			vec4 vertices[3];
			vec4 transformed_vertices[3];

			int out_of_bounds[6] = { 0, 0, 0, 0, 0, 0 };
			for (int j = 0; j < 3; ++j) {
				vertices[j] = gl_in[j].gl_Position - light_pos;
				transformed_vertices[j] = shadow_transforms[i] * vertices[j];

				if (transformed_vertices[j].x >  transformed_vertices[j].w) ++out_of_bounds[0];
				if (transformed_vertices[j].x < -transformed_vertices[j].w) ++out_of_bounds[1];
				if (transformed_vertices[j].y >  transformed_vertices[j].w) ++out_of_bounds[2];
				if (transformed_vertices[j].y < -transformed_vertices[j].w) ++out_of_bounds[3];
				if (transformed_vertices[j].z >  transformed_vertices[j].w) ++out_of_bounds[4];
				if (transformed_vertices[j].z < -transformed_vertices[j].w) ++out_of_bounds[5];
			}

			bool in_frustum = true;
			for (int k = 0; k < 6; ++k)
				if (out_of_bounds[k] == 3)
					in_frustum = false;

			if (in_frustum) {
				gl_Layer = i + l * 6;
				for (int j = 0; j < 3; ++j) {
					vout.uv = vin[j].uv;
					vout.matIdx = vin[j].matIdx;
					vout.position = vertices[j];

					gl_Position = transformed_vertices[j];

					EmitVertex();
				}

				EndPrimitive();
			}
		}
	}
}
