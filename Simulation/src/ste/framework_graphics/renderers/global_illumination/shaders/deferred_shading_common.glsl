
#include <light.glsl>

struct deferred_shading_shadow_maps {
	samplerCubeArrayShadow shadow_depth_maps;
	samplerCubeArray shadow_maps;
	sampler2DArrayShadow directional_shadow_depth_maps;
	sampler2DArray directional_shadow_maps;
};

struct deferred_material_microfacet_luts { 
	sampler2D microfacet_refraction_fit_lut;
	sampler2DArray microfacet_transmission_fit_lut;
};

struct deferred_material_ltc_luts { 
	sampler2D ltc_ggx_fit;
	sampler2D ltc_ggx_amplitude;
};

struct deferred_atmospherics_luts { 
	sampler2DArray atmospheric_optical_length_lut;
	sampler3D atmospheric_scattering_lut;
	sampler3D atmospheric_mie0_scattering_lut;
	sampler3D atmospheric_ambient_lut;
};

struct light_shading_parameters {
	light_descriptor ld;
	uint light_id;
	uint ll_id;

	// Light distance and direction
	float l_dist;
	vec3 l;

	// Light illuminance arriving at shaded sample
	vec3 cd_m2;
};

struct fragment_shading_parameters {
	// Screen space coords
	ivec2 coords;

	// View space position, normal, tangent, bi-tangent
	vec3 p;
	vec3 n;
	vec3 t;
	vec3 b;

	// Normalized ray from fragment to eye, view space
	vec3 v;

	// World space position, normal
	vec3 world_position;
	vec3 world_normal;
	vec3 world_v;
};
