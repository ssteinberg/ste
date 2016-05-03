
#include "hdr_common.glsl"

const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const int max_active_lights_per_frame = 32;

struct light_descriptor {
	vec3 position_direction;	uint32_t type;
	vec3 diffuse;				float luminance;

	float radius;
	float effective_range;

	uint32_t shadow_face_mask;
	float minimal_luminance;
};

float light_attenuation_factor(light_descriptor ld, float dist) {
	if (ld.type == LightTypeDirectional)
		return 1;
	else {
		float a = max(.001f, dist / ld.radius);
		float f = 1.f / (a*a);

		return f;
	}
}

float light_effective_range(light_descriptor ld) {
	if (ld.type == LightTypeDirectional)
		return +1.0f / .0f;		// Infinity
	else
		return ld.effective_range;
}

vec4 light_transform(mat4 mv, mat3 rmv, light_descriptor ld) {
	vec4 transform;
	if (ld.type == LightTypeSphere)
		transform = mv * vec4(ld.position_direction.xyz, 1);
	else
		transform.xyz = rmv * ld.position_direction.xyz;

	return transform;
}

float light_calculate_effective_range(light_descriptor ld, float min_lum) {
	float l = min_lum;
	return ld.radius * (sqrt(ld.luminance / l) - 1.f);
}
