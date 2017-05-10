
#type geometry
#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

struct buffer_glyph_descriptor {
	uint width;
	uint height;
	int start_y;
	int start_x;

	uint sampler_idx;
};

in vs_out {
	uvec2 position;
	vec3 color;
	vec3 stroke_color;
	uint drawId;
	float weight;
	float stroke_width;
	float size;
} vin[];

out geo_out {
	flat vec3 color;
	flat vec3 stroke_color;
	flat float weight;
	flat float stroke_width;
	flat uint drawId;
} vout;
layout(location = 0) out vec2 tex_coords;

layout(std430, set = 0, binding = 0) restrict readonly buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

layout(push_constant) uniform push_constants_t {
	vec2 fb_size;
};

void main() {
	vec2 pos = mix(vec2(-1,1), vec2(1,-1), vec2(vin[0].position) / fb_size);

	buffer_glyph_descriptor g = glyphs[vin[0].drawId];

	vec2 size = vin[0].size / fb_size;

	vout.color = vin[0].color;
	vout.stroke_color = vin[0].stroke_color;
	vout.drawId = vin[0].drawId;
	vout.weight = vin[0].weight;
	vout.stroke_width = vin[0].stroke_width;
	tex_coords = vec2(0, 0);
	gl_Position = vec4(pos + size * vec2(g.start_x, g.start_y), 0, 1);
	EmitVertex();
	
	vout.color = vin[0].color;
	vout.stroke_color = vin[0].stroke_color;
	vout.drawId = vin[0].drawId;
	vout.weight = vin[0].weight;
	vout.stroke_width = vin[0].stroke_width;
	tex_coords = vec2(1, 0);
	gl_Position = vec4(pos + size * vec2(g.start_x + g.width, g.start_y), 0, 1);
	EmitVertex();
	
	vout.color = vin[0].color;
	vout.stroke_color = vin[0].stroke_color;
	vout.drawId = vin[0].drawId;
	vout.weight = vin[0].weight;
	vout.stroke_width = vin[0].stroke_width;
	tex_coords = vec2(0, 1);
	gl_Position = vec4(pos + size * vec2(g.start_x, g.start_y - g.height), 0, 1);
	EmitVertex();
	
	vout.color = vin[0].color;
	vout.stroke_color = vin[0].stroke_color;
	vout.drawId = vin[0].drawId;
	vout.weight = vin[0].weight;
	vout.stroke_width = vin[0].stroke_width;
	tex_coords = vec2(1, 1);
	gl_Position = vec4(pos + size * vec2(g.start_x + g.width, g.start_y - g.height), 0, 1);
	EmitVertex();

	EndPrimitive();
}

