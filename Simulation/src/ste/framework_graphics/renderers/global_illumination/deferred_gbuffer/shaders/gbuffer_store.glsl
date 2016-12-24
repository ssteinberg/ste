
#include "gbuffer.glsl"
#include "pack.glsl"
#include "girenderer_transform_buffer.glsl"

void gbuffer_store(float depth,
				   vec2 UV,
				   vec2 dUVdx,
				   vec2 dUVdy,
				   vec3 N,
				   vec3 T,
				   int matIdx,
				   ivec2 frag_coords) {
	float dUVdx16 = uintBitsToFloat(packFloat2x16(f16vec2(dUVdx)));
	float dUVdy16 = uintBitsToFloat(packFloat2x16(f16vec2(dUVdy)));

	uint Npack = packSnorm2x16(norm3x32_to_snorm2x32(N));
	uint Tpack = packSnorm2x16(norm3x32_to_snorm2x32(T));

	mat2x4 data;
	data[0] = vec4(depth, uintBitsToFloat(matIdx), dUVdx16, dUVdy16);
	data[1] = vec4(UV, uintBitsToFloat(Npack), uintBitsToFloat(Tpack));

	uint idx = frag_coords.y * backbuffer_size().x + frag_coords.x;
	gbuffer[idx].data = data;
}
