
#type compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

const int light_buffers_first = 2;
#include "light.glsl"
#include "shadow.glsl"

layout(binding = 5) uniform sampler2D wposition_tex;
layout(binding = 6) uniform sampler2D wnormal_tex;
layout(binding = 7) uniform sampler2D frag_depth_tex;
layout(binding = 8) uniform samplerCubeArray shadow_depth_maps;
layout(r16f, binding = 0) uniform image2DArray penumbra_layers;
layout(r16f, binding = 1) uniform image2D z_buffer;

uniform float far;
uniform float half_over_tan_fov_over_two;

void main() {
	vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(penumbra_layers).xy);

	vec3 n = textureLod(wnormal_tex, uv, 0).xyz;
	vec3 w_pos = textureLod(wposition_tex, uv, 0).xyz;
	float frag_depth = textureLod(frag_depth_tex, uv, 0).x;

	for (int i = 0; i < light_buffer.length(); ++i) {
		vec3 l_pos = light_buffer[i].position_direction.xyz;
		vec3 shadow_v = w_pos - l_pos;
		float d = dot(n, -shadow_v);

		float frag_out = .0f;

		if (d > .0f) {
			float l_radius = light_buffer[i].radius;
			float dist = length(w_pos - l_pos);

			bool shadowed;
			float w_penumbra = shadow_penumbra_width(shadow_depth_maps, i, shadow_v, l_radius, dist, far, shadowed);

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
