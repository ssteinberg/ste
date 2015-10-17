
#type geometry
#version 450

layout (points) in;
uniform vec2 fb_size;

layout (triangle_strip, max_vertices = 4) out;

in vs_out {
	vec4 color;
	float coc;
} vin[];

out geo_out {
	vec4 color;
	vec2 st;
} vout;

const float size = 80;

void main() {
	vec4 pos = gl_in[0].gl_Position;
	
	vout.color = vin[0].color;
	float coc = vin[0].coc;

	vout.st = vec2(0, 0);
    gl_Position = pos + vec4(size * vec2(-coc, -coc) / fb_size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(1, 0);
    gl_Position = pos + vec4(size * vec2( coc, -coc) / fb_size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(0, 1);
    gl_Position = pos + vec4(size * vec2(-coc,  coc) / fb_size, 0, 0);
    EmitVertex();
	
	vout.st = vec2(1, 1);
    gl_Position = pos + vec4(size * vec2( coc,  coc) / fb_size, 0, 0);
    EmitVertex();

    EndPrimitive();
}
  
