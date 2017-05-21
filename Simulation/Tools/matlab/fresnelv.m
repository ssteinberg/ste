function [R] = fresnelv(n, l, r)

    x = dot(n,l);

    Rs = @(x) abs((x - r * sqrt(1 - (1 - x * x) / (r*r))) / (x + r * sqrt(1 - (1 - x * x) / (r*r))))^2;
    Rp = @(x) abs((r * x - sqrt(1 - (1 - x * x) / (r*r))) / (r * x + sqrt(1 - (1 - x * x) / (r*r))))^2;
    
    R = (Rp(x) + Rs(x)) / 2;

end
