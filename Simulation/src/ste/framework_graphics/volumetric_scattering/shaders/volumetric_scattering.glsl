
#include "common.glsl"

const int volumetric_scattering_tile_size = 8;
const int volumetric_scattering_depth_tiles = 512;

float volumetric_scattering_depth_for_tile(int z) {
	return float(z) / float(volumetric_scattering_depth_tiles);
}

float volumetric_scattering_tile_for_depth(float d) {
	return d * float(volumetric_scattering_depth_tiles);
}

float volumetric_scattering_particle_density(vec3 w_pos) {
	return .005f;
}

float volumetric_scattering_scattering_coefficient(float density, float thickness) {
	return 0.005f * density * thickness;
}

float volumetric_scattering_absorption_coefficient(float density, float thickness) {
	return .000001f * density * thickness;
}

float volumetric_scattering_phase(vec3 l_dir, vec3 v_dir) {
	float g = .15f;

	float t = dot(l_dir, v_dir);
	float g2 = g*g;
	float denom = pow(1.f + g2 - 2.f*g*t, 3.f / 2.f);
	return ((1.f - g2) / denom) / (4.f * pi);
}

vec4 volumetric_scattering_load_inscattering_transmittance(sampler3D volume, vec2 frag_coords, float depth) {
	vec2 xy = frag_coords / float(volumetric_scattering_tile_size) / textureSize(volume, 0).xy;
	vec3 p = vec3(xy, depth);
	return texture(volume, p);
}
