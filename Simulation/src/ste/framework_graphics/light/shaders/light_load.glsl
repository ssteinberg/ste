
#include "light.glsl"

vec3 light_incidant_ray(light_descriptor ld, vec3 position) {
	if (ld.type == LightTypeDirectional) return -ld.transformed_position;
	else return ld.transformed_position - position;
}

vec3 light_irradiance(light_descriptor ld, float dist) {
	float attenuation_factor = light_attenuation_factor(ld, dist);
	float incident_radiance = max(ld.luminance * attenuation_factor - ld.minimal_luminance, .0f);
	return ld.diffuse * max(0.f, incident_radiance);
}
