
#type vert
#version 450

layout(location = 0) in vec4 vert_glyph_size;
layout(location = 1) in vec4 col;
layout(location = 2) in float weight;

uniform mat4 proj;

out vs_out {
	vec4 color;
	int drawId;
	float size;
	float weight;
} vout;

void main( void ) {	
	vout.drawId = int(vert_glyph_size.z);
	vout.size = vert_glyph_size.w;
	vout.weight = weight;
	vout.color = col;
    gl_Position = proj * vec4(vert_glyph_size.xy, 0, 1);
}

