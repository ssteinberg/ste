
#type compute
#version 450
#extension GL_NV_gpu_shader5 : require

layout(local_size_x = 32, local_size_y = 32) in;

#include "girenderer_matrix_buffer.glsl"
#include "light.glsl"
#include "shadow.glsl"
#include "gbuffer.glsl"

layout(std430, binding = 2) restrict readonly buffer light_data {
	light_descriptor light_buffer[];
};

layout(shared, binding = 6) restrict readonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};
layout(r32ui, binding = 7) restrict readonly uniform uimage2D gbuffer_ll_heads;

layout(binding = 8) uniform samplerCubeArray shadow_depth_maps;
layout(r16f, binding = 0) restrict uniform image2DArray penumbra_layers;
layout(r16f, binding = 1) restrict uniform image2D z_buffer;

uniform float near;
uniform float half_over_tan_fov_over_two;

void main() {
	mat4 inverse_view_matrix = transpose(view_matrix_buffer.transpose_inverse_view_matrix);
	mat4 transpose_view_matrix = transpose(view_matrix_buffer.view_matrix);

	ivec2 coords = ivec2(vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(penumbra_layers).xy) * gbuffer_size(gbuffer_ll_heads));

	g_buffer_element frag = gbuffer_load(gbuffer_ll_heads, coords);
	vec3 n = (transpose_view_matrix * vec4(frag.N, 1)).xyz;
	vec3 w_pos = (inverse_view_matrix * vec4(frag.P, 1)).xyz;
	float frag_depth = ;

	for (int i = 0; i < light_buffer.length(); ++i) {
		vec3 l_pos = light_buffer[i].position_direction.xyz;
		vec3 shadow_v = w_pos - l_pos;
		float d = dot(n, -shadow_v);

		float frag_out = .0f;

		if (d > .0f) {
			float l_radius = light_buffer[i].radius;
			float dist = length(w_pos - l_pos);

			bool shadowed;
			float w_penumbra = shadow_penumbra_width(shadow_depth_maps, i, shadow_v, l_radius, dist, shadowed);

			if (shadowed) {
				float anisotropy = dist / d;
				float d_screen = half_over_tan_fov_over_two;
				float w_screen_penumbra = w_penumbra * d_screen / frag_depth * anisotropy;

				frag_out = max(w_screen_penumbra, .05f);
			}
		}

		imageStore(penumbra_layers, ivec3(gl_GlobalInvocationID.xy, i), frag_out.xxxx);
	}

	imageStore(z_buffer, ivec2(gl_GlobalInvocationID.xy), frag_depth.xxxx);
}
