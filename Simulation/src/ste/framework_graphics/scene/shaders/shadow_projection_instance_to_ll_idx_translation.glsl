
#include "light.glsl"

const int shadow_proj_id_to_ll_id_table_size = max_active_lights_per_frame;
struct shadow_projection_instance_to_ll_idx_translation {
	uint16_t ll_idx[shadow_proj_id_to_ll_id_table_size];
};
