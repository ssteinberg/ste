function [] = refract_fitting()

DIM = 256;
N = 40;
Rmin = .2;
Rmax = 3.2;

% roughness = .045001;
% r = 2.9999;

fprintf('%s - Starting\n', datetime('now'));

data = zeros(DIM,DIM,6);
for i = 1:DIM
    roughness = (i-1) / (DIM-1);
    alpha = roughness * roughness;
    
    D = @(x) alpha * alpha / (pi * ((alpha*alpha - 1) * x*x + 1)^2);
    
    for j = 1:DIM
        r = (j-1) / (DIM-1) * (Rmax - Rmin) + Rmin;
        s = asin(min(1, r));

        x = zeros(N,1);
        y = zeros(N,1);
        w = zeros(N,1);
        
        x(1) = 0;
        y(1) = 0;
        w(1) = 100;
        for k = 2:N
            v = (k-1) / (N-1) * pi / 2;
            
            t = @(theta) s * sin(pi/2 * (theta - (v - s)) / s);
            f = @(theta) D(cos(theta)) * sin(abs(2 * theta)) / 2 * ...
                integral(@(phi) refract_clamp(v, theta, phi, r), -t(theta), t(theta), 'ArrayValued', true, 'RelTol', 1e-5, 'AbsTol', 1e-6);

            a = max(-pi/2, v - s);
            b = min( pi/2, v + s);
            
            if (r ~= 1)
                Fv = integral(f, a, b, 'ArrayValued', true, 'RelTol', 5e-4, 'AbsTol', 1e-5);
                o = Fv / norm(Fv);
            
                if (abs(o(2)) > 1e-8)
                    fprintf('bad vector...');
                end
            end

            x(k) = v;
            if (r ~= 1)
                if (norm(Fv) > 0)
                    y(k) = o(1);
                else
                    y(k) = y(k-2) + 2*(y(k-1) - y(k-2));
                end
            else
                y(k) = -sin(v);
            end
            w(k) = 1;
        end
        w(N) = 10;

%         ft = fittype('max(0, a1*exp(-((x-x0)^n)/(a2)) + b1*exp(-((x-x1)^n)/(b2)) + c1*exp(-((y-y0)^n)/(c2))) + d0 + d1*y + d2*y^2 + d3*y^3 + d4*y^4 + d5*y^5','problem','n','independent',{'x', 'y'},'dependent',{'z'});
%         opts = fitoptions(ft);
%         opts.Robust = 'Bisquare'; %LAR
%         opts.MaxIter = 4000;
%         opts.MaxFunEvals = 3000;

        ft = fitoptions('gauss2');
        ft.Weights = w;
        sf = fit(x, y, 'gauss2', ft);%ft, opts, 'problem', 2);
%         plot(sf, x, y, 'o');
        data(i,j,:) = coeffvalues(sf);
        
        fprintf('%s - progress %.2f\n', datetime('now'), 100.0*(j*i)/(DIM^2));
    end
end

save('refract_fitting_data.mat', 'data');

end
