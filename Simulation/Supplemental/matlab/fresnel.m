function [R] = fresnel(theta_v, theta, phi, r)

    m = [sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)];
    l = [sin(theta_v), 0, cos(theta_v)];
    
    x = dot(m,l);

    Rs = @(x) abs((x - r * sqrt(r*r - 1 + x * x)/r) / (x + r * sqrt(r*r - 1 + x * x)/r))^2;
    Rp = @(x) abs((r * x - sqrt(r*r - 1 + x * x)/r) / (r * x + sqrt(r*r - 1 + x * x)/r))^2;
    
    R = Rp(x) + Rs(x) / 2;

end

