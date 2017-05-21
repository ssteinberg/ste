function [v, is_tir] = refract(l, n, r)

    t = 1/r;
    c = dot(l,n);
    
    x = 1 - t*t*(1-c*c);
    is_tir = x < 0;
    v = -t*l + (t*c - sqrt(x))*n;

end
