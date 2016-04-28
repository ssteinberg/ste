
const int LightTypeSphere = 0;
const int LightTypeDirectional = 1;

const float light_cutoff = 0.001f;
const float light_min_effective_lum_ratio = 0.00001;
const int max_active_lights_per_frame = 32;

struct light_descriptor {
	vec3 position_direction;	uint32_t type;
	vec3 diffuse;				float luminance;

	float radius;
	float effective_range;

	uint32_t shadow_face_mask;
	float _unused;
};

float light_attenuation_factor(light_descriptor ld, float dist) {
	if (ld.type == LightTypeDirectional)
		return 1;
	else {
		float a = max(.001f, dist / ld.radius);
		return a*a;
	}
}

float light_min_effective_luminance(light_descriptor ld) {
	return max(light_min_effective_lum_ratio * ld.luminance, light_cutoff);
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
