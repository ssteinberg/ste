
#type vert
#version 450

layout(location = 1) in vec3 vert;

out vec4 gl_Position;
out vs_out {
	flat int invocation_id;
} vout;

void main() {
	vout.invocation_id = gl_InstanceID;
	gl_Position = vec4(vert, 1);
}
