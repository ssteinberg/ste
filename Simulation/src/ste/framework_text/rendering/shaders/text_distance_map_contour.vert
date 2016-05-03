
#type vert
#version 450

layout(location = 0) in vec4 vert_glyph_size;
layout(location = 1) in vec4 col;
layout(location = 2) in vec4 stroke_col;
layout(location = 3) in float stroke_width;
layout(location = 4) in float weight;

uniform mat4 proj;

out vs_out {
	vec4 color;
	vec4 stroke_color;
	int drawId;
	float weight;
	float stroke_width;
	float size;
} vout;

void main() {
	vout.drawId = int(vert_glyph_size.z);
	vout.size = vert_glyph_size.w;
	vout.weight = weight;
	vout.color = col;
	vout.stroke_color = stroke_col;
	vout.stroke_width = stroke_width;

    gl_Position = proj * vec4(vert_glyph_size.xy, 0, 1);
}
