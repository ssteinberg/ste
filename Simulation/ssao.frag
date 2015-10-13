
#type frag
#version 440

out float gl_FragColor;

layout(binding = 0) uniform sampler2D normal_tex;
layout(binding = 2) uniform sampler2D noise_tex;
layout(binding = 3) uniform sampler2DArray depth_layer;

const float pi = 3.1415926535897932384626433832795;
uniform float min_step_size = 2;
uniform float max_step_size = 15;
uniform int points = 4;
uniform int steps = 6;
uniform mat4 proj_inv;
uniform float near = 1.0f;
uniform float far = 1000.0f;

float rand(vec2 s) { return fract(sin(dot(s.xy ,vec2(12.9898,78.233))) * 43758.5453); }

vec3 unproject(vec2 frag_coords, float depth) {
	vec2 p = 2.0f * frag_coords/textureSize(normal_tex,0) - float(1).xx;

	float Q = (far + near) / (far - near);
	float T = 2.0f * far * near / (far - near);
	float eye_z = -(depth * (far - near) + near);
	float d = (Q*eye_z + T) / eye_z;
	
	vec4 t = proj_inv * vec4(p, d*2.0f - 1.0f, 1);
	return t.xyz / t.w;
}

void main() {
	vec3 normal = texelFetch(normal_tex, ivec2(gl_FragCoord.xy), 0).xyz;
	float depth = texelFetch(depth_layer, ivec3(gl_FragCoord.xy, 0), 0).x;
	vec3 position = unproject(gl_FragCoord.xy, depth);

	vec3 offset = position + normal * .25f;

	vec2 dir = textureLod(noise_tex, vec2(gl_FragCoord.xy / textureSize(noise_tex,0)), 0).xy;
	float step_size = rand(gl_FragCoord.xy) * (max_step_size - min_step_size) + min_step_size;

	float occ = .0f;
	const float angle = 2*pi/float(points);
	mat2 rotation_mat = mat2(cos(angle), sin(angle),
							 -sin(angle),cos(angle));

	for (int i=0; i<points; ++i) {
		float max_d = .0f;
		float s = step_size;
		vec2 tc = gl_FragCoord.xy;
		for (int j=1; j<steps; ++j, s*=1.5f) {
			vec2 p = tc + s * dir + float(1).xx * pow(2,j-1);

			float sample_d = 0;
			float diff = 1;
			for (int k=0; k<3; ++k) {
				float layer_d = textureLod(depth_layer, vec3(p / textureSize(normal_tex, 0), k), j).x;
				if (layer_d==1.0f)
					break;
				float layer_diff = abs(depth-layer_d);
				if (layer_diff<diff) {
					diff = layer_diff;
					sample_d = layer_d;
				}
				else
					break;
				if (layer_d>depth)
					break;
			}
			vec3 screen_p = unproject(p, sample_d);

			vec3 v = screen_p - offset;
			float l = length(v);
			v /= l;
			float d = max(0, (dot(v,normal) - .2f) / .8f) / pow(l,1.85f);

			max_d = max(max_d, d);
		}
		
		occ += max_d;
		dir = rotation_mat * dir;
	}
	 
	occ /= float(points);
	occ = pow(occ,.4f);
	
	gl_FragColor = occ;
}
