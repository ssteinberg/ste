
#include "hdr_common.glsl"
#include "quaternion.glsl"
#include "dual_quaternion.glsl"

const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const int max_active_lights_per_frame = 32;

struct light_descriptor {
	vec4 position_range;
	vec3 diffuse;	float luminance;

	float radius;
	uint32_t type;

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
		return ld.position_range.w;
}

vec3 light_transform(dual_quaternion transform, light_descriptor ld) {
	return ld.type == LightTypeSphere ?
				dquat_mul_vec(transform, ld.position_range.xyz) :
				quat_mul_vec(transform.real, ld.position_range.xyz);
}

float light_calculate_effective_range(light_descriptor ld, float min_lum) {
	float l = min_lum;
	return ld.radius * (sqrt(ld.luminance / l) - 1.f);
}
