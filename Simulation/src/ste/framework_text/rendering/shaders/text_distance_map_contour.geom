
#type geometry
#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

struct buffer_glyph_descriptor {
	int width;
	int height;
	int start_y;
	int start_x;
	//layout(bindless_sampler) sampler2D tex_handler;
	int sampler_idx;
};

in vs_out {
	vec4 color;
	vec4 stroke_color;
	int drawId;
	float weight;
	float stroke_width;
	float size;
} vin[];

out geo_out {
	vec4 color;
	vec4 stroke_color;
	float weight;
	float stroke_width;
	vec2 st;
	flat int drawId;
} vout;

layout(std430, binding = 0) restrict readonly buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

uniform fb_size_uniform_t {
	vec2 fb_size;
};

void main() {
	vec4 pos = gl_in[0].gl_Position;

	buffer_glyph_descriptor g = glyphs[vin[0].drawId];

	float size = vin[0].size;

	vout.color = vin[0].color;
	vout.stroke_color = vin[0].stroke_color;
	vout.drawId = vin[0].drawId;
	vout.weight = vin[0].weight;
	vout.stroke_width = vin[0].stroke_width;

	vout.st = vec2(0, 0);
	gl_Position = pos + vec4(size * vec2(g.start_x, g.start_y) / fb_size, 0, 0);
	EmitVertex();

	vout.st = vec2(1, 0);
	gl_Position = pos + vec4(size * vec2(g.start_x + g.width, g.start_y) / fb_size, 0, 0);
	EmitVertex();

	vout.st = vec2(0, 1);
	gl_Position = pos + vec4(size * vec2(g.start_x, g.start_y + g.height) / fb_size, 0, 0);
	EmitVertex();

	vout.st = vec2(1, 1);
	gl_Position = pos + vec4(size * vec2(g.start_x + g.width, g.start_y + g.height) / fb_size, 0, 0);
	EmitVertex();

	EndPrimitive();
}

