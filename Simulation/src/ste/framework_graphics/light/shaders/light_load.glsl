
#include "light.glsl"

vec3 light_incidant_ray(light_descriptor ld, vec3 position) {
	if (ld.type == LightTypeDirectional) return -ld.transformed_position;
	else return ld.transformed_position - position;
}
