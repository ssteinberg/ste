function [y] = erf_approximation(x)
p = .47047;
a1 = .3480242;
a2 = -.0958798;
a3 = .7478556;

t = 1 / (1 + p*abs(x));

y = sign(x) * (1 - (a1*t + a2*t^2 + a3*t^3)*exp(-x^2));
end

