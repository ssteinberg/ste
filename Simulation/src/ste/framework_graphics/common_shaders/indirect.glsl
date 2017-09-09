
struct indirect_multi_draw_elements_command {
	uint index_count;
	uint instance_count;
	uint first_index;
	int vertex_offset;
	uint base_instance;
};

struct indirect_draw_arrays_command {
	uint count;
	uint instance_count;
	uint first_index;
	uint base_instance;
};

struct indirect_dispatch_command {
	uint num_groups_x;
	uint num_groups_y;
	uint num_groups_z;
};
