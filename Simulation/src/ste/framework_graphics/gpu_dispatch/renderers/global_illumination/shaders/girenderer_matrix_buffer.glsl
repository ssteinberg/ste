
struct matrix_buffer_struct {
	mat4 view_matrix;
	mat4 transpose_inverse_view_matrix;
	mat4 projection_view_matrix;
};

layout(std430, binding = 20) restrict readonly buffer matrix_buffer_data {
	matrix_buffer_struct view_matrix_buffer;
};
