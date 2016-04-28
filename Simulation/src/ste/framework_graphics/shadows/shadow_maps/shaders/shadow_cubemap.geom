
#type geometry
#version 450
#extension GL_NV_gpu_shader5 : require

layout(triangles) in;
layout(triangle_strip, max_vertices=15) out;

#include "light.glsl"

in vs_out {
	vec3 normal;
	vec2 uv;
	flat int matIdx;
} vin[];

out frag_in {
	vec2 uv;
	flat int matIdx;
} vout;

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(std430, binding = 4) restrict readonly buffer ll_counter_data {
	uint32_t ll_counter;
};
layout(std430, binding = 5) restrict readonly buffer ll_data {
	uint16_t ll[];
};

layout(std430, binding = 9) restrict readonly buffer projection_data {
	mat4 shadow_transforms[];
};

uniform float far;

void process(int face, int l, vec4 vertices[3]) {
	vec4 transformed_vertices[3];

	int out_of_bounds[6] = { 0, 0, 0, 0, 0, 0 };
	for (int j = 0; j < 3; ++j) {
		transformed_vertices[j] = shadow_transforms[face] * vertices[j];

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
		gl_Layer = face + l * 6;
		for (int j = 0; j < 3; ++j) {
			vout.uv = vin[j].uv;
			vout.matIdx = vin[j].matIdx;

			gl_Position = transformed_vertices[j];

			EmitVertex();
		}

		EndPrimitive();
	}
}

void main() {
	for (int i = 0; i < ll_counter; ++i) {
		light_descriptor ld = light_buffer[ll[i]];

		vec4 light_pos = vec4(ld.position_direction.xyz, 0);
		float light_range = min(ld.effective_range, far);
		float light_range2 = light_range * light_range;

		vec3 N = (vin[0].normal + vin[1].normal + vin[2].normal) / 3.f;
		vec3 V = light_pos.xyz - gl_in[0].gl_Position.xyz;

		if (dot(N,V) < 0) {
			vec4 vertices[3];

			int out_of_range = 0;
			for (int j = 0; j < 3; ++j) {
				vec4 P = gl_in[j].gl_Position - light_pos;
				if (dot(P.xyz, P.xyz) >= light_range2)
					++out_of_range;

				vertices[j] = P;
			}

			if (out_of_range < 3) {
				for (int face = 0; face < 6; ++face)
					process(face, i, vertices);
			}
		}
	}
}
