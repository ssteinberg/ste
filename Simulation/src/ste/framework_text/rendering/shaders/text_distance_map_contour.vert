
#type vert
#version 450

layout(location = 0) in vec4 vert_glyph_size;
layout(location = 1) in vec4 col;
layout(location = 2) in vec4 stroke_col;
layout(location = 3) in float stroke_width;
layout(location = 4) in float weight;

uniform proj_uniform_t {
	mat4 proj;
};

out vs_out {
	vec2 position;
	vec4 color;
	vec4 stroke_color;
	int drawId;
	float weight;
	float stroke_width;
	float size;
} vout;

void main() {
	//gl_Position = proj * vec4(vert_glyph_size.xy, 0, 1);
	vout.position = vert_glyph_size.xy / vec2(1920, 1080) * 2.f - vec2(1);
	vout.drawId = int(vert_glyph_size.z);
	vout.size = vert_glyph_size.w;
	vout.weight = weight;
	vout.color = col;
	vout.stroke_color = stroke_col;
	vout.stroke_width = stroke_width;
}
