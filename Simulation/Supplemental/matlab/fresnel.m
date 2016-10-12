function [R] = fresnel(theta_v, theta, phi, r)

    m = [sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)];
    l = [sin(theta_v), 0, cos(theta_v)];
    
    R = fresnelv(m,l,r);

end
