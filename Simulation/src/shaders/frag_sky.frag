
#type frag
#version 450
#extension GL_NV_gpu_shader5 : require

layout(early_fragment_tests) in;

#include "gbuffer.glsl"
#include "material.glsl"

layout(shared, binding = 6) restrict writeonly buffer gbuffer_data {
	g_buffer_element gbuffer[];
};

#include "gbuffer_store.glsl"

in vec2 frag_texcoords;
in vec3 frag_position;

layout(binding = 0) uniform sampler2D sky_tex;

uniform int material;

void main() {
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;

	gbuffer_store(gl_FragCoord.z,
				  vec2(uv),
				  dFdx(uv),
				  dFdy(uv),
				  vec3(0,0,0),
				  vec3(0,0,0),
				  material,
				  ivec2(gl_FragCoord.xy));
}
