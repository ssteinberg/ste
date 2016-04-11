
#type compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

const int light_buffers_first = 2;
#include "light.glsl"
#include "shadow.glsl"

layout(r32f, binding = 0) uniform image2DArray penumbra_layers;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 1) uniform sampler2D position_tex;
layout(binding = 8) uniform samplerCubeArray shadow_depth_maps;

uniform float far;
uniform mat4 inv_view_model;

void main() {
	vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(penumbra_layers).xy);

	vec3 p = texture(position_tex, uv).xyz;
	vec3 n = texture(normal_tex, uv).xyz;

	vec3 w_pos = (inv_view_model * vec4(p, 1)).xyz;
	float dist = length(w_pos);

	for (int i = 0; i < light_buffer.length(); ++i) {
		float obscurance = shadow_obscurance(shadow_depth_maps, i, w_pos, light_buffer[i].position_direction.xyz, dist, far);
		imageStore(penumbra_layers, ivec3(gl_GlobalInvocationID.xy, i), vec4(obscurance * 100.f, 0.f, 0.f, 0.f));
	}
}
