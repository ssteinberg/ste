
#include "hdr_common.glsl"
#include "quaternion.glsl"
#include "dual_quaternion.glsl"

const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const int max_active_lights_per_frame = 32;

struct light_descriptor {
	vec3 position;	float radius;
	vec3 diffuse;	float luminance;

	uint32_t type;

	uint32_t shadow_face_mask;
	float minimal_luminance;

	float _reserved;

	vec3 transformed_position;	float effective_range;
};

float light_attenuation_factor(light_descriptor ld, float dist) {
	if (ld.type == LightTypeDirectional)
		return 1;
	else {
		float a = max(.0f, dist / ld.radius);
		float f = 1.f / (1.f + a*a);

		return f;
	}
}

float light_effective_range(light_descriptor ld) {
	if (ld.type == LightTypeDirectional)
		return +1.0f / .0f;		// Infinity
	else
		return ld.effective_range;
}

vec3 light_transform(dual_quaternion transform, light_descriptor ld) {
	return ld.type == LightTypeSphere ?
				dquat_mul_vec(transform, ld.position) :
				quat_mul_vec(transform.real, ld.position);
}

float light_calculate_effective_range(light_descriptor ld, float min_lum) {
	float l = min_lum;
	return ld.radius * (sqrt(ld.luminance / l) - 1.f);
}
