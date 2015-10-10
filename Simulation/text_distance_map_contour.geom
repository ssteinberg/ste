
#version 450
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_bindless_texture : require
#extension GL_NV_gpu_shader5 : require

struct buffer_glyph_descriptor {
	int16_t width;
	int16_t height;
	int16_t start_y;
	int16_t start_x;
	uint64_t tex_handler;
};

layout (points) in;
uniform vec2 fb_size;

layout (triangle_strip, max_vertices = 4) out;

in vs_out {
	vec4 color;
	int drawId;
	float size;
	float weight;
} vin[];

out geo_out {
	vec4 color;
	vec2 st;
	flat int drawId;
	float weight;
} vout;

layout(std430, binding = 0) buffer glyph_data {
	buffer_glyph_descriptor glyphs[];
};

void main() {
	vec4 pos = gl_in[0].gl_Position;
	buffer_glyph_descriptor g = glyphs[vin[0].drawId];

	float size = vin[0].size;
	
	vout.drawId = vin[0].drawId;
	vout.color = vin[0].color;
	vout.weight = vin[0].weight;

	vout.st = vec2(0, 0);
    gl_Position = pos + vec4(vec2(g.start_x, g.start_y) / fb_size * size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(1, 0);
    gl_Position = pos + vec4(vec2(g.start_x + g.width, g.start_y) / fb_size * size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(0, 1);
    gl_Position = pos + vec4(vec2(g.start_x, g.start_y + g.height) / fb_size * size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(1, 1);
    gl_Position = pos + vec4(vec2(g.start_x + g.width, g.start_y + g.height) / fb_size * size,  0, 0);
    EmitVertex();

    EndPrimitive();
}
  
