
#type compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

const int light_buffers_first = 2;
#include "light.glsl"
#include "shadow.glsl"

layout(binding = 5) uniform sampler2D wposition_tex;
layout(binding = 7) uniform samplerCubeArray shadow_depth_maps;
layout(r16f, binding = 0) uniform image2DArray penumbra_layers;

uniform float far;

void main() {
	vec2 uv = vec2(gl_GlobalInvocationID.xy) / vec2(imageSize(penumbra_layers).xy);

	vec3 w_pos = texture(wposition_tex, uv).xyz;

	for (int i = 0; i < light_buffer.length(); ++i) {
		vec3 l_pos = light_buffer[i].position_direction.xyz;
		float dist = length(w_pos - l_pos);

		float obscurance = shadow_obscurance(shadow_depth_maps, i, w_pos, l_pos, dist, far);
		imageStore(penumbra_layers, ivec3(gl_GlobalInvocationID.xy, i), vec4(obscurance, 0.f, 0.f, 0.f));
	}
}
