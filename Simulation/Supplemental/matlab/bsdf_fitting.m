function [] = transmission_fitting()

DIM = 256;
N = 40;
Rmin = .2;
Rmax = 3.2;
    
D = @(x, alpha) alpha * alpha / (pi * ((alpha*alpha - 1) * x*x + 1)^2);

roughness = .25001;
r = 1.;

fprintf('%s - Starting\n', datetime('now'));

% data = zeros(DIM,DIM,6);
% for i = 1:DIM
%     roughness = (i-1) / (DIM-1);
    alpha = roughness * roughness;
    
%     for j = 1:DIM
%         r = (j-1) / (DIM-1) * (Rmax - Rmin) + Rmin;
        s = asin(min(1, r));
        
        F = @(x) (Rp(x) + Rs(x)) / 2;

        x = single(zeros(N,1));
        y = single(zeros(N,1));
        for k = 1:N
            v = (k-1) / (N-1) * pi / 2;
            
            t = @(theta) s * sin(pi/2 * (theta - (v - s)) / s); 
            f = @(theta) D(cos(theta)) * sin(abs(2 * theta)) / 2 * ...
                integral(@(phi) (1 - fresnel(v, theta, phi, r)), -t(theta), t(theta), 'ArrayValued', true, 'RelTol', 1e-4, 'AbsTol', 1e-4);
        
            a = max(-pi/2, v - s);
            b = min( pi/2, v + s);
            
            Fv = integral(f, a, b, 'ArrayValued', true, 'RelTol', 1e-4, 'AbsTol', 1e-4);

            x(k) = cos(v);
            y(k) = Fv;
        end

%         ft = fittype('max(0, a1*exp(-((x-x0)^n)/(a2)) + b1*exp(-((x-x1)^n)/(b2)) + c1*exp(-((y-y0)^n)/(c2))) + d0 + d1*y + d2*y^2 + d3*y^3 + d4*y^4 + d5*y^5','problem','n','independent',{'x', 'y'},'dependent',{'z'});
%         opts = fitoptions(ft);
%         opts.Robust = 'Bisquare'; %LAR
%         opts.MaxIter = 4000;
%         opts.MaxFunEvals = 3000;

        sf = fit(x, y, 'gauss2');%ft, opts, 'problem', 2);
%         data(i,j,:) = coeffvalues(sf);
        plot(sf,x,y,'o');
%     end
%     
%     fprintf('%s - progress %.2f\n', datetime('now'), 100.0*i/DIM);
% end

% save('transmission_fitting_data.mat', 'data');

end
