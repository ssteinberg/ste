
#include "gbuffer.glsl"
#include "pack.glsl"
#include "girenderer_transform_buffer.glsl"

g_buffer_element gbuffer_encode(float depth,
								vec2 UV,
								vec2 dUVdx,
								vec2 dUVdy,
								vec3 N,
								vec3 T,
								int matIdx) {
	float dUVdx16 = uintBitsToFloat(packHalf2x16(dUVdx));
	float dUVdy16 = uintBitsToFloat(packHalf2x16(dUVdy));

	uint Npack = packSnorm2x16(norm3x32_to_snorm2x32(N));
	uint Tpack = packSnorm2x16(norm3x32_to_snorm2x32(T));

	g_buffer_element g_frag;
	g_frag.data[0] = vec4(depth, uintBitsToFloat(matIdx), dUVdx16, dUVdy16);
	g_frag.data[1] = vec4(UV, uintBitsToFloat(Npack), uintBitsToFloat(Tpack));

	return g_frag;
}
