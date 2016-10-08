function [v] = refract_clamp(theta_v, theta, phi, r)

    m = [sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)];
    l = [sin(theta_v), 0, cos(theta_v)];
    
    t = 1/r;
    c = dot(l,m);
    o = -t*l + (t*c - sqrt(1 - t*t*(1-c*c)))*m;
    
    if (o(3) < 0)
        v = o / norm(o);
    else
        v = [0 0 0];
    end

end

