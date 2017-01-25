
#include "light.glsl"
#include "light_cascades.glsl"

struct drawid_to_lightid_ttl_entry {
	uint data;
};

const int s_drawid_to_lightid_ttl_size = max_active_lights_per_frame;
struct drawid_to_lightid_ttl {
	drawid_to_lightid_ttl_entry entries[s_drawid_to_lightid_ttl_size];
};

const int d_drawid_to_lightid_ttl_size = max_active_directional_lights_per_frame;
struct d_drawid_to_lightid_ttl {
	drawid_to_lightid_ttl_entry entries[d_drawid_to_lightid_ttl_size];
};

uint translate_drawid_to_ll_idx(drawid_to_lightid_ttl_entry entry) {
	return entry.data & 0xFFFF;
}

uint translate_drawid_to_light_idx(drawid_to_lightid_ttl_entry entry) {
	return entry.data >> 16;
}

drawid_to_lightid_ttl_entry create_drawid_ttl_entry(uint ll_idx, uint light_idx) {
	drawid_to_lightid_ttl_entry entry;
	entry.data = (ll_idx & 0xFFFF) | (light_idx << 16);
	return entry;
}
