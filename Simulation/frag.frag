
#version 440

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;

layout(location = 0) out vec3 o_frag_color;
layout(location = 1) out vec3 o_frag_position;
layout(location = 2) out vec3 o_frag_normal;

layout(binding = 0, r32i) uniform iimage2D depth_layer0;
layout(binding = 1, r32i) uniform iimage2D depth_layer1;
layout(binding = 2, r32i) uniform iimage2D depth_layer2;

layout(binding = 0) uniform sampler2D tex;
layout(binding = 1) uniform sampler2D normal_map;
layout(binding = 2) uniform sampler2D mask;

uniform bool has_mask = false;
uniform bool has_normal_map = false;

void main() {
	if (has_mask && texture(mask, frag_texcoords).x<.5f) {
		discard;
		return;
	}
	
	//vec3 frag_normal = normalize(cross(dPdx,dPdy));
	o_frag_normal = normalize(frag_normal);
	
	if (has_normal_map) {
		vec3 nm = texture(normal_map, frag_texcoords).rgb * 2.0f - float(1.0f).xxx;
		nm.y = -nm.y;
		
		vec3 dPdx = dFdx(frag_position);
		vec3 dPdy = dFdy(frag_position);
		vec2 dUVdx = dFdx(frag_texcoords);
		vec2 dUVdy = dFdy(frag_texcoords);
		
		vec3 T = dPdx * dUVdx.x + dPdy * dUVdy.x;
		vec3 B = dPdx * dUVdx.y + dPdy * dUVdy.y;

		float invmax = inversesqrt(max(dot(T,T),dot(B,B)));
		mat3 TBN = mat3(T * invmax, B * invmax, o_frag_normal);

		o_frag_normal = normalize(TBN * nm);
	}
	
	o_frag_color = texture(tex, frag_texcoords).rgb;
	o_frag_position = frag_position;
	
	int int_from_float = floatBitsToInt(1.0f - gl_FragCoord.z);
	int t;
	if ((t=imageAtomicMax(depth_layer0, ivec2(gl_FragCoord.xy), int_from_float)) < int_from_float) int_from_float = t;
	if ((t=imageAtomicMax(depth_layer1, ivec2(gl_FragCoord.xy), int_from_float)) < int_from_float) int_from_float = t;
	imageAtomicMax(depth_layer2, ivec2(gl_FragCoord.xy), int_from_float);
}
