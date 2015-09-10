
#version 440

layout(binding = 0) out float o_d0;
layout(binding = 1) out float o_d1;
layout(binding = 2) out float o_d2;

layout(binding = 3) uniform sampler2DArray depth_layer;

uniform int level;

float op(vec4 depths) { return min(min(depths.x,depths.y),min(depths.z,depths.w)); }

void main() {
	vec4 depths0 = textureGather(depth_layer, vec3(gl_FragCoord.xy/textureSize(depth_layer,level).xy, 0), level);
	o_d0 = op(depths0);
	
	vec4 depths1 = textureGather(depth_layer, vec3(gl_FragCoord.xy/textureSize(depth_layer,level).xy, 1), level);
	o_d1 = op(depths1);
	
	vec4 depths2 = textureGather(depth_layer, vec3(gl_FragCoord.xy/textureSize(depth_layer,level).xy, 2), level);
	o_d2 = op(depths2);
}
