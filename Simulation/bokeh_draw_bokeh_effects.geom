
#type geometry
#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vs_out {
	vec4 color;
	float coc;
} vin[];

out fp_in {
	vec4 color;
	vec2 uv;
} vout;

uniform vec2 fb_size;
const float size = 40;

void main() {
	float coc = vin[0].coc;

	vout.color = vin[0].color;

	vout.uv = vec2(0,0);
	gl_Position = gl_in[0].gl_Position + vec4(vec2(-size * coc, -size * coc) / fb_size, 0, 0);
	EmitVertex();

	vout.uv = vec2(1,0);
	gl_Position = gl_in[0].gl_Position + vec4(vec2( size * coc, -size * coc) / fb_size, 0, 0);
	EmitVertex();

	vout.uv = vec2(0,1);
	gl_Position = gl_in[0].gl_Position + vec4(vec2(-size * coc,  size * coc) / fb_size, 0, 0);
	EmitVertex();

	vout.uv = vec2(1,1);
	gl_Position = gl_in[0].gl_Position + vec4(vec2( size * coc,  size * coc) / fb_size, 0, 0);
	EmitVertex();

	EndPrimitive();
}

