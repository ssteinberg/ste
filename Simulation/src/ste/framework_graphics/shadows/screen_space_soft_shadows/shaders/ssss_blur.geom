
#type geometry
#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vs_out {
	flat int invocation_id;
} vin[];

void main() {
	gl_Layer = vin[0].invocation_id;

	for (int j = 0; j < 3; ++j) {
		gl_Position = gl_in[j].gl_Position;
		EmitVertex();
	}

	EndPrimitive();
}