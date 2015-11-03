
#type frag
#version 450

in vec2 frag_texcoords;
in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_tangent;
in float frag_depth;

layout(location = 0) out vec4 o_frag_color;
layout(location = 1) out vec3 o_frag_position;
layout(location = 2) out vec3 o_frag_normal;
layout(location = 3) out float o_frag_z;
layout(location = 4) out vec3 o_frag_tangent;
layout(location = 5) out uint o_frag_mat_idx;

uniform vec3 light_diffuse;
uniform float light_luminance;

void main() {
	vec2 uv = frag_texcoords;
	vec3 P = frag_position;
	vec3 n = normalize(frag_normal);
	vec3 t = normalize(frag_tangent);
	vec3 b = cross(t, n);
	
	o_frag_tangent = t;
	o_frag_normal = n;
	
	o_frag_color = vec4(light_diffuse * light_luminance, 1);

	o_frag_position = P;
	o_frag_z = frag_depth;
	o_frag_mat_idx = -1;
}
