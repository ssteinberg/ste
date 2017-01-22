
#include "light.glsl"
#include "light_cascades.glsl"

const int shadow_proj_id_to_ll_id_table_size = max_active_lights_per_frame;
struct shadow_projection_instance_to_ll_idx_translation {
	uint ll_idx[shadow_proj_id_to_ll_id_table_size];
};

const int directional_shadow_proj_id_to_ll_id_table_size = max_active_directional_lights_per_frame;
struct directional_shadow_projection_instance_to_ll_idx_translation {
	uint ll_idx[directional_shadow_proj_id_to_ll_id_table_size];
};
