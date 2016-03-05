
struct mesh_descriptor {
	mat4 model, transpose_inverse_model;
	int matIdx;
	int _unused[3];
};

layout(std430, binding = 1) buffer mesh_data {
	mesh_descriptor mesh_descriptor_buffer[];
};
