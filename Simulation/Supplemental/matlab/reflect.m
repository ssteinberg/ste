function [v] = reflect(l, n)

    v = l - 2.0 * dot(n, l) * n;

end
