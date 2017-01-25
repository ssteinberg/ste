
#include "atmospherics_descriptor.glsl"
#include "light_transport.glsl"
#include "common.glsl"

layout(std430, binding = 22) restrict readonly buffer atmospherics_descriptor_buffer {
	atmospherics_descriptor atmospherics_descriptor_data;
};


/*
*	Returns the atmospheric center in world position
*/
vec3 atmospherics_center() {
	return atmospherics_descriptor_data.center_radius.xyz;
}

/*
*	Returns the atmospheric lower radius (planet radius) in world position
*/
float atmospherics_sea_level_radius() {
	return atmospherics_descriptor_data.center_radius.w;
}

/*
*	Returns the altitude of a point
*
*	@param w_pos	World position
*/
float atmospherics_altitude(vec3 w_pos) {
	return max(.0f, length(w_pos - atmospherics_center()) - atmospherics_sea_level_radius());
}

/*
*	Returns the atmospheric air density at world position in kg/m^3
*
*	@param w_pos	World position
*/
float atmospherics_air_density(vec3 w_pos) {
	float altitude = atmospherics_altitude(w_pos);
	return atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor_data, altitude);
}

/*
*	Returns the atmospheric aerosols density at world position in kg/m^3
*
*	@param w_pos	World position
*/
float atmospherics_aerosol_density(vec3 w_pos) {
	float altitude = atmospherics_altitude(w_pos);
	return atmospherics_descriptor_pressure_mie(atmospherics_descriptor_data, altitude);
}

/*
*	Returns the atmospheric air optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_air(vec3 P0, vec3 P1, 
									  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_rayleigh(atmospherics_descriptor_data, 
														   P0, P1, 
														   atmospheric_optical_length_lut);
}

/*
*	Returns the atmospheric aerosol optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_aerosol(vec3 P0, vec3 P1, 
										  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_mie(atmospherics_descriptor_data, 
													  P0, P1, 
													  atmospheric_optical_length_lut);
}

/*
*	Returns the atmospheric air optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_fast_air(vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_fast_rayleigh(atmospherics_descriptor_data, 
														   P0, P1);
}

/*
*	Returns the atmospheric aerosol optical length
*
*	@param P0	Start world position
*	@param P1	End world position
*/
float atmospherics_optical_length_fast_aerosol(vec3 P0, vec3 P1) {
	return atmospherics_descriptor_optical_length_fast_mie(atmospherics_descriptor_data, 
													  P0, P1);
}

/*
*	Returns the atmospheric air optical length for a ray from P0 in direction V.
*
*	@param P0	Start world position
*	@param V	Normalized ray direction
*/
float atmospherics_optical_length_ray_air(vec3 P0, vec3 V, 
										  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_ray_rayleigh(atmospherics_descriptor_data, 
															   P0, V, 
															   atmospheric_optical_length_lut);
}

/*
*	Returns the atmospheric aerosol optical length for a ray from P0 in direction V.
*
*	@param P0	Start world position
*	@param V	Normalized ray direction
*/
float atmospherics_optical_length_ray_aerosol(vec3 P0, vec3 V, 
											  sampler2DArray atmospheric_optical_length_lut) {
	return atmospherics_descriptor_optical_length_ray_mie(atmospherics_descriptor_data, 
														  P0, V, 
														  atmospheric_optical_length_lut);
}

/*
*	Returns the Mie extinction coefficient in m^-1
*/
float atmospherics_mie_extinction_coeffcient() {
	return atmospherics_descriptor_data.mie_scattering_coefficient + atmospherics_descriptor_data.mie_absorption_coefficient;
}

/*
*	Returns the Mie extinction coefficient in m^-1
*/
vec3 atmospherics_rayleigh_extinction_coeffcient() {
	return atmospherics_descriptor_data.rayleigh_scattering_coefficient;
}

/*
*	Calculates the total atmospheric extinction along a path.
*
*	@param P0	Start world position
*	@param P1	End world position
*/
vec3 extinct(vec3 P0, vec3 P1, 
			 sampler2DArray atmospheric_optical_length_lut) {
	float tr = atmospherics_optical_length_fast_air(P0, P1);
	float tm = atmospherics_optical_length_fast_aerosol(P0, P1);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}
/*
*	Calculates the total atmospheric extinction along a multi-point route.
*	The route is the lines (P0,P1) and (P1,P2).
*
*	@param P0	World position
*	@param P1	World position
*	@param P2	World position
*/
vec3 extinct(vec3 P0, vec3 P1, vec3 P2, 
			 sampler2DArray atmospheric_optical_length_lut) {
	float tr = atmospherics_optical_length_fast_air(P0, P1) + 
			   atmospherics_optical_length_fast_air(P1, P2);
	float tm = atmospherics_optical_length_fast_aerosol(P0, P1) +
			   atmospherics_optical_length_fast_aerosol(P1, P2);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}
/*
*	Calculates the total atmospheric extinction along a multi-point route.
*	The route is the lines (P0,P1), (P1,P2), etc.
*
*	@param P0	World position
*	@param P1	World position
*	@param P2	World position
*	@param P3	World position
*/
vec3 extinct(vec3 P0, vec3 P1, vec3 P2, vec3 P3, 
			 sampler2DArray atmospheric_optical_length_lut) {
	float tr = atmospherics_optical_length_fast_air(P0, P1) + 
			   atmospherics_optical_length_fast_air(P1, P2) + 
			   atmospherics_optical_length_fast_air(P2, P3);
	float tm = atmospherics_optical_length_fast_aerosol(P0, P1) +
			   atmospherics_optical_length_fast_aerosol(P1, P2) +
			   atmospherics_optical_length_fast_aerosol(P2, P3);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates the total atmospheric extinction along a ray.
*
*	@param P0	Start point in world coordinates
*	@param V	Normalized ray direction in world coordinates
*/
vec3 extinct_ray(vec3 P0, vec3 V, 
				 sampler2DArray atmospheric_optical_length_lut) {
	float tr = atmospherics_optical_length_ray_air(P0, V, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_ray_aerosol(P0, V, atmospheric_optical_length_lut);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates the total atmospheric extinction along a path.
*	The path is the line (P0, P1) and then the ray from P1 in direction V.
*
*	@param P0	World position
*	@param P1	World position
*	@param V	Normalized ray direction in world coordinates
*/
vec3 extinct_ray(vec3 P0, vec3 P1, vec3 V, 
				 sampler2DArray atmospheric_optical_length_lut) {
	float tr = atmospherics_optical_length_ray_air(P1, V, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_fast_air(P0, P1);
	float tm = atmospherics_optical_length_ray_aerosol(P1, V, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_fast_aerosol(P0, P1);
	vec3 t = atmospherics_rayleigh_extinction_coeffcient() * tr +
			 vec3(atmospherics_mie_extinction_coeffcient()) * tm;
	return beer_lambert(t);
}

/*
*	Calculates a single atmospheric scattering event and the total atmospheric extinction along a path.
*
*	@param P0		Start world position
*	@param P1		Scatter point, as world position
*	@param P2		End world position
*	@param len		Length, in meters, of the scattering event sample
*/
vec3 scatter(vec3 P0, 
			 vec3 P1, 
			 vec3 P2, 
			 float len, 
			 sampler2DArray atmospheric_optical_length_lut) {	
	vec3 i = normalize(P0 - P1);
	vec3 o = normalize(P2 - P1);
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	
	float altitude = atmospherics_altitude(P1);
	float density_m = atmospherics_descriptor_pressure_mie(atmospherics_descriptor_data, altitude);
	float density_r = atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor_data, altitude);

	vec3 scatter_coefficient = density_m * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   density_r * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
	
	vec3 scattered_intensity = scatter_coefficient * len;
	vec3 extinction = extinct(P0, P1, P2, atmospheric_optical_length_lut);

	return scattered_intensity * extinction;
}

/*
*	Calculates a single atmospheric scattering event and the total atmospheric extinction along a path.
*	The path is the line (P0, P1) and then the ray from P1 in direction V.
*
*	@param P0		Start world position
*	@param P1		Scatter point, as world position
*	@param V		Normalized ray direction in world coordinates
*	@param len		Length, in meters, of the scattering event sample
*/
vec3 scatter_ray(vec3 P0,
				 vec3 P1, 
				 vec3 V, 
				 float len, 
				 sampler2DArray atmospheric_optical_length_lut) {
	vec3 i = normalize(P0 - P1);
	vec3 o = V;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	
	float altitude = atmospherics_altitude(P1);
	float density_m = atmospherics_descriptor_pressure_mie(atmospherics_descriptor_data, altitude);
	float density_r = atmospherics_descriptor_pressure_rayleigh(atmospherics_descriptor_data, altitude);

	vec3 scatter_coefficient = density_m * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   density_r * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
	
	vec3 scattered_intensity = scatter_coefficient * len;
	vec3 extinction = extinct_ray(P0, P1, V, atmospheric_optical_length_lut);

	return scattered_intensity * extinction;
}

/*
*	Returns the scattering indicatrix which relates the relative luminance of a sky element to its angular 
*	distance from the light source.
*	Parameters are for the "CIE Standard Clear Sky, low luminance turbidity" model.
*	See "Spatial distribution of daylight - CIE standard general sky", CIE DS 011.2/E:2002.
*
*	@param x		Angular distance
*	@param cos_x	Precomputed cosine of the angular distance
*/
float cie_scattering_indicatrix(float x, float cos_x) {
	return 1.f + 10.f * (exp(-3.f * x) - exp(-3.f * pi_over_2)) + 0.45f * cos_x*cos_x;
}
/*
*	Returns the normalization factor for the scattering indicatrix, i.e. the indicatrix at angular distance 0.
*/
float cie_scattering_indicatrix_normalizer() {
	return cie_scattering_indicatrix(.0f, 1.0f);
}

/*
*	Calculates the multiple-scattered irradiance reaching an obsever in the atmosphere, given viewing 
*	direction and light source direction. 
*	Uses a precomputed LUT for calculation.
*
*	@param P		Observer world position
*	@param L		Light direction
*	@param V		Viewing direction
*/
vec3 atmospheric_scatter(vec3 P, vec3 L, vec3 V, 
						 sampler3D atmospheric_scattering_lut,
						 sampler3D atmospheric_mie0_scattering_lut) {
	vec3 C = atmospherics_descriptor_data.center_radius.xyz;
	float r = atmospherics_descriptor_data.center_radius.w;

	// Compute the up vector, i.e. the from the center of the atmosphere to viewer position
	vec3 Y = P - C;
	float Ylen = length(Y);
	vec3 N = Y / Ylen;
	
	// Compute height in atmosphere, view-zenith angle and sun-zenith angle.
	float h = Ylen - r;
	float cos_phi = dot(N, V);
	float cos_delta = dot(N, -L);
	
	// And convert those into LUT lookup indices
	float x = _atmospheric_height_to_lut_idx(h, atmospherics_descriptor_data.Hr_max);
	float y = _atmospheric_view_zenith_to_lut_idx(cos_phi);
	float z = _atmospheric_sun_zenith_to_lut_idx(cos_delta);

	// Read multiple-scatter and Mie single-scatter lookup values
	vec3 scatter = texture(atmospheric_scattering_lut, vec3(x,y,z)).rgb;
	vec3 m0 = texture(atmospheric_mie0_scattering_lut, vec3(x,y,z)).rgb;
	
	// Finally account for light-view azimuth by using the CIE scattering indicatrix
	float cos_gamma = dot(V, -L);
	float gamma = acos(cos_gamma);
	float indicatrix = cie_scattering_indicatrix(gamma, cos_gamma);
	
	// Finally compute multiple-scattering azimuth modulator and the high-res Mie phase function sample
	float p = indicatrix / cie_scattering_indicatrix_normalizer();
	float p_mie = cornette_shanks_phase_function(V, L, atmospherics_descriptor_data.phase);

	return scatter * p + 
		   m0 * p_mie;
}

/*
*	Calculates the multiple-scattered ambient irradiance reaching a point.
*	Uses a precomputed LUT for calculation.
*
*	@param P		Observer world position
*	@param L		Light direction
*	@param V		Viewing direction
*/
vec3 atmospheric_ambient(vec3 P, float NdotL, vec3 L, 
						 sampler3D atmospheric_ambient_lut) {
	vec3 C = atmospherics_descriptor_data.center_radius.xyz;
	float r = atmospherics_descriptor_data.center_radius.w;

	// Compute the up vector, i.e. the from the center of the atmosphere to viewer position
	vec3 Y = P - C;
	float Ylen = length(Y);
	vec3 U = Y / Ylen;
	
	// Compute height in atmosphere, view-zenith angle and sun-zenith angle.
	float h = Ylen - r;
	float cos_delta = dot(U, -L);
	
	// And convert those into LUT lookup indices
	float x = _atmospheric_height_to_lut_idx(h, atmospherics_descriptor_data.Hr_max);
	float y = _atmospheric_sun_zenith_to_lut_idx(cos_delta);
	float z = _atmospheric_ambient_NdotL_to_lut_idx(NdotL);

	return texture(atmospheric_ambient_lut, vec3(x,y,z)).rgb;
}
