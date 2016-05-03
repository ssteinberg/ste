
#include "light.glsl"

vec3 light_incidant_ray(light_descriptor ld, int i, vec3 position) {
	if (ld.type == LightTypeDirectional) return -light_transform_buffer[i].xyz;
	else return light_transform_buffer[i].xyz - position;
}
