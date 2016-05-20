
#include "common.glsl"
#include "project.glsl"

const int volumetric_scattering_tile_size = 8;
const int volumetric_scattering_depth_tiles = 256;

const float volumetric_scattering_ka = 0.025f;
const float volumetric_scattering_kb = 1.06f;

float volumetric_scattering_depth_for_tile(int t) {
	float z = -volumetric_scattering_ka * pow(volumetric_scattering_kb, float(t));
	return -1.f / z;
}

float volumetric_scattering_tile_for_depth(float d) {
	float z = -1.f / d;
	return log(-z / volumetric_scattering_ka) / log(volumetric_scattering_kb);
}

float volumetric_scattering_zcoord_for_depth(float d) {
	return volumetric_scattering_tile_for_depth(d) / float(volumetric_scattering_depth_tiles);
}

float volumetric_scattering_particle_density(vec3 w_pos) {
	return .05f;
}

float volumetric_scattering_scattering_coefficient(float density, float thickness) {
	return 0.00055f * density * thickness;
}

float volumetric_scattering_absorption_coefficient(float density, float thickness) {
	return .0000001f * density * thickness;
}

float volumetric_scattering_phase(vec3 l_dir, vec3 v_dir, float phase1, float phase2, float phase3) {
	float t = dot(l_dir, v_dir);
	float denom = pow(phase2 - phase3 * t, 3.f / 2.f);
	return phase1 / denom;
}

vec4 volumetric_scattering_load_inscattering_transmittance(sampler3D volume, vec2 frag_coords, float depth) {
	vec2 xy = frag_coords / float(volumetric_scattering_tile_size) / textureSize(volume, 0).xy;
	vec3 p = vec3(xy, volumetric_scattering_zcoord_for_depth(depth));
	return texture(volume, p);
}

vec3 volumetric_scattering(sampler3D volume, vec3 rgb, vec2 frag_coords, float depth) {
	vec4 vol_sam = volumetric_scattering_load_inscattering_transmittance(volume,
																		 frag_coords,
																		 depth);
	return rgb * vol_sam.a + vol_sam.rgb;
}