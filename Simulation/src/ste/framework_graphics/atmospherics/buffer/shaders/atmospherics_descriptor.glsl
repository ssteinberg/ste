
struct atmospherics_descriptor {
	// Sea level athmospheric pressure, kPa
	float ro0;
	// Sea level temperature, Kelvin
	float T0;
	// Gravitation acceleration, m/s^2
	float g;
	// Temperature lapse rate, https://en.wikipedia.org/wiki/Adiabatic_lapse_rate
	// K/m
	float L;
	// Ideal (universal) gas constant, J/(mol·K)
	float R;
	// Molar mass of the atmospheric air, kg/mol
	float M;

	// Phase coefficient for the Mie scattering phase function
	float phase;

	// Precomputed values
	float M_over_R;
	float gM_over_RL;

	float _ununsed[3];
};

/*
*	Returns the density given pressure and temperature
*
*	@param p	Pressure, kPa 
*	@param t	Temperature, Kelvin
*/
float atmospherics_density(atmospherics_descriptor desc, float p, float t) {
	return p * desc.M_over_R / t;
}

/*
*	Returns the pressure given temperature
*
*	@param t	Temperature, Kelvin
*/
float atmospherics_pressure(atmospherics_descriptor desc, float t) {
	float e = desc.gM_over_RL;
	return desc.ro0 * pow(t / desc.T0, e);
}

/*
*	Returns the temperature at height h
*	
*	@param h	Height in meters
*/
float atmospherics_temperature_at_altitude(atmospherics_descriptor desc, float h) {
	return desc.T0 - desc.L*h;
}

/*
*	Returns the pressure at height h
*
*	@param h	Height in meters
*/
float atmospherics_pressure_at_altitude(atmospherics_descriptor desc, float h) {
	float t = atmospherics_temperature_at_altitude(desc, h);
	return pressure(t);
}

/*
*	Returns the density at height h
*
*	@param h	Height in meters
*/
float atmospherics_density_at_altitude(atmospherics_descriptor desc, float h) {
	float t = atmospherics_temperature_at_altitude(desc, h);
	float p = atmospherics_pressure(desc, t);
	return atmospherics_density(desc, p, t);
}
