
#include "light.glsl"

struct shadow_projection_instance_to_ll_idx_translation {
	uint16_t ll_idx[max_active_lights_per_frame];
};
