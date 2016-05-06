
struct IndirectMultiDrawElementsCommand {
	uint count;
	uint instance_count;
	uint first_index;
	uint base_vertex;
	uint base_instance;
};

struct IndirectDrawArraysCommand {
	uint count;
	uint prim_count;
	uint first_index;
	uint base_instance;
};

struct IndirectDispatchCommand {
	uint num_groups_x;
	uint num_groups_y;
	uint num_groups_z;
};
