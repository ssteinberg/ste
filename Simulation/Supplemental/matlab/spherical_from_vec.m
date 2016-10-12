function [ theta, phi ] = spherical_from_vec(v)
	x2y2 = 1 - v(3)*v(3);

	theta = atan(sqrt(x2y2) / v(3));
	phi = atan2(v(2), v(1));
end
