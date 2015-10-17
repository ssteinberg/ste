
#type frag
#version 440

out vec4 gl_FragColor;

layout(binding = 0) uniform sampler2D tex;

const float center_weight = 0.20236;
const vec4 weights = vec4(	0.179044,	0.124009,	0.067234,	0.028532);

void main() {
	vec4 l3 = texelFetch(tex, ivec2(gl_FragCoord.xy) - ivec2(0,4), 0);
	vec4 l2 = texelFetch(tex, ivec2(gl_FragCoord.xy) - ivec2(0,3), 0);
	vec4 l1 = texelFetch(tex, ivec2(gl_FragCoord.xy) - ivec2(0,2), 0);
	vec4 l0 = texelFetch(tex, ivec2(gl_FragCoord.xy) - ivec2(0,1), 0);
	vec4 c = texelFetch(tex, ivec2(gl_FragCoord.xy), 0);
	vec4 r0 = texelFetch(tex, ivec2(gl_FragCoord.xy) + ivec2(0,1), 0);
	vec4 r1 = texelFetch(tex, ivec2(gl_FragCoord.xy) + ivec2(0,2), 0);
	vec4 r2 = texelFetch(tex, ivec2(gl_FragCoord.xy) + ivec2(0,3), 0);
	vec4 r3 = texelFetch(tex, ivec2(gl_FragCoord.xy) + ivec2(0,4), 0);
			
	vec4 lw = vec4(l0.w, l1.w, l2.w, l3.w) * weights;
	vec4 rw = vec4(r0.w, r1.w, r2.w, r3.w) * weights;
	float totalw = dot(lw, vec4(1)) + dot(rw, vec4(1)) + center_weight;
	vec4 blur = (l3 * lw.w + l2 * lw.z + l1 * lw.y + l0 * lw.x +
				 r3 * rw.w + r2 * rw.z + r1 * rw.y + r0 * rw.x + 
				 c * center_weight) / totalw;

	gl_FragColor = blur;
}
