function [v] = refract_clamp(l, n, r)

    t = 1/r;
    c = dot(l,n);
    v = -t*l + (t*c - sqrt(1 - t*t*(1-c*c)))*n;

end
