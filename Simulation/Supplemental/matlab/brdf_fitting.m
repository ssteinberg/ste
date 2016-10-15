function [] = brdf_fitting()

DIM = 256;
N = 40;
M = 90;
Samples = 16e+5;
Rmin = .2;
Rmax = 3.2;

% GGX Smith 'Lambda' function
Lambda = @(theta, roughness) -.5 + .5 * sqrt(1 + (roughness^2 * tan(theta))^2);

% NDF and its corresponding PDF, CDF and inverse CDF in spherical
% coordinates
D = @(theta, roughness) roughness^4 / (pi * ((roughness^4 - 1) * cos(theta)^2 + 1)^2);
PDF = @(theta, roughness) 2 * roughness^4 * cos(theta) * sin(theta) / (((roughness^4 - 1) * cos(theta)^2 + 1)^2);
CDF = @(theta, roughness) roughness^4 / (((roughness^4 - 1) * cos(theta))^2 + (roughness^4 - 1)) - 1 / (roughness^4 - 1);
CDFinv = @(E, roughness) atan(roughness^2 * sqrt(E/(1-E)));

% Gaussian height distribution and its CDF and inverse CDF
Psigma = @(roughness) (roughness^2 * 1.0);
P1 = @(h, roughness) 1 / sqrt(2*pi*Psigma(roughness)^2) * exp(-h*h / (2*Psigma(roughness)^2));
C1 = @(h, roughness) .5*(1 + erf(h / (Psigma(roughness) * sqrt(2))));
C1inv = @(p, roughness) sqrt(2)*Psigma(roughness)*erfinv(2*p - 1);

roughness = .25001;
r = 1.5;
v = pi/4;

fprintf('%s - Starting\n', datetime('now'));

% data = zeros(DIM,DIM,6);
% for i = 1:DIM
%     roughness = (i-1) / (DIM-1);
    
%     for j = 1:DIM
%         r = (j-1) / (DIM-1) * (Rmax - Rmin) + Rmin;
            
%         for k = 1:N
%             v = (k-1) / (N-1) * pi / 2;
            [ brdf, btdf,x,y ] = generate_brdf(0,@(x)D(x,roughness),...
                                                 @(x)PDF(x,roughness),...
                                                 @(x)CDF(x,roughness),...
                                                 @(x)CDFinv(x,roughness),...
                                                 @(x)Lambda(x,roughness),...
                                                 @(x)P1(x,roughness),...
                                                 @(x)C1(x,roughness),...
                                                 @(x)C1inv(x,roughness),...
                                                 M,Samples,v,r,roughness);

%             ft = fittype('max(0, a1*exp(-((x-x0)^n)/(a2)) + b1*exp(-((x-x1)^n)/(b2)) + c1*exp(-((y-y0)^n)/(c2))) + d0 + d1*y + d2*y^2 + d3*y^3 + d4*y^4 + d5*y^5','problem','n','independent',{'x', 'y'},'dependent',{'z'});
%             opts = fitoptions(ft);
%             opts.Robust = 'Bisquare'; %LAR
%             opts.MaxIter = 4000;
%             opts.MaxFunEvals = 3000;

            sf = fit([x, y], brdf, 'poly44');%ft, opts, 'problem', 2);
%             data(i,j,:) = coeffvalues(sf);
            plot(sf,[x,y],brdf,'o');
%         end
%     end
%     
%     fprintf('%s - progress %.2f\n', datetime('now'), 100.0*i/DIM);
% end

% save('transmission_fitting_data.mat', 'data');

end
