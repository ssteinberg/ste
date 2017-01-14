
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
	float tr = atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut);
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
	float tr = atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_air(P1, P2, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P1, P2, atmospheric_optical_length_lut);
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
	float tr = atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_air(P1, P2, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_air(P2, P3, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P1, P2, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P2, P3, atmospheric_optical_length_lut);
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
			   atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_ray_aerosol(P1, V, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut);
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
*	@param I		Precomputed direction of light incident ray, originating from P1 to P2, normalized.
*	@param L		Precomputed direction of light incident ray, originating from P1 to P0, normalized.
*	@param len		Length, in meters, of the scattering event sample
*/
vec3 scatter(vec3 P0, 
			 vec3 P1, 
			 vec3 P2, 
			 vec3 I, 
			 vec3 L,
			 float len, 
			 sampler2DArray atmospheric_optical_length_lut) {	
	float tr = atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_air(P1, P2, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P1, P2, atmospheric_optical_length_lut);
			   
	vec3 Tr = tr * atmospherics_rayleigh_extinction_coeffcient();
	vec3 Tm = vec3(tm) * atmospherics_mie_extinction_coeffcient();

	vec3 i = I;
	vec3 o = L;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	vec3 scatter_coefficient = beer_lambert(Tm) * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   beer_lambert(Tr) * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
							   
	float particle_density = atmospherics_air_density(P1);

	return scatter_coefficient * len * particle_density;
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
	vec3 I = normalize(P2 - P1);
	vec3 L = normalize(P0 - P1);
	return scatter(P0,
				   P1,
				   P2,
				   I, L,
				   len, 
				   atmospheric_optical_length_lut);
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
	float tr = atmospherics_optical_length_ray_air(P1, V, atmospheric_optical_length_lut) + 
			   atmospherics_optical_length_air(P0, P1, atmospheric_optical_length_lut);
	float tm = atmospherics_optical_length_ray_aerosol(P1, V, atmospheric_optical_length_lut) +
			   atmospherics_optical_length_aerosol(P0, P1, atmospheric_optical_length_lut);
			   
	vec3 Tr = tr * atmospherics_rayleigh_extinction_coeffcient();
	vec3 Tm = vec3(tm) * atmospherics_mie_extinction_coeffcient();

	vec3 i = normalize(P1 - P0);
	vec3 o = V;
	float p_mie = cornette_shanks_phase_function(i, o, atmospherics_descriptor_data.phase);
	float p_rayleigh = rayleigh_phase_function(i, o);
	vec3 scatter_coefficient = beer_lambert(Tm) * p_mie * atmospherics_descriptor_data.mie_scattering_coefficient.xxx + 
							   beer_lambert(Tr) * p_rayleigh * atmospherics_descriptor_data.rayleigh_scattering_coefficient;
							   
	float particle_density = atmospherics_air_density(P1);
	
	return scatter_coefficient * len * particle_density;
}

/*
*	Calculates the multiple-scattered irradiance reaching an obsever in the atmosphere, given viewing 
*	direction and light source direction. 
*	Uses a LUT for calculation.
*
*	@param P		Observer world position
*	@param L		Light direction
*	@param V		Viewing direction
*/
vec3 atmospheric_scatter(vec3 P, vec3 L, vec3 V, 
						 sampler2DArray atmospheric_optical_length_lut,
						 sampler3D atmospheric_scattering_lut) {
	vec3 C = atmospherics_descriptor_data.center_radius.xyz;
	float r = atmospherics_descriptor_data.center_radius.w;
	vec3 invL = -L;

	vec3 Y = P - C;
	float Ylen = length(Y);
	vec3 N = Y / Ylen;
	
	float h = Ylen - r;
	float cos_phi = dot(N, V);
	float cos_delta = dot(N, invL);
	
	float x = _atmospheric_height_to_lut_idx(h, atmospherics_descriptor_data.Hr_max);
	float y = _atmospheric_view_zenith_to_lut_idx(cos_phi);
	float z = _atmospheric_sun_zenith_to_lut_idx(cos_delta);
	vec4 lut = texture(atmospheric_scattering_lut, vec3(x,y,z));

	float p_mie = cornette_shanks_phase_function(V, invL, atmospherics_descriptor_data.phase);

	vec3 scatter = lut.rgb;
	float m0 = p_mie * lut.a;

	return scatter + m0.xxx;
}
