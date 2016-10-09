function [ output_args ] = Untitled2( input_args )
% ior1 = 9.1;
% ior2 = 4.4;

N = 256;
M = 256;
Rmin = .25;
Rmax = 4.0;

x = zeros(M,1);
y = zeros(N,1);
z = zeros(N,M);
for j = 1:N
    r = (j-1) / (N-1) * (Rmax - Rmin) + Rmin;
    s = asin(min(1, r));

    Rs = @(x) abs((x - r * sqrt(1 - (1 - x * x) / (r*r))) / (x + r * sqrt(1 - (1 - x * x) / (r*r))))^2;
    Rp = @(x) abs((r * x - sqrt(1 - (1 - x * x) / (r*r))) / (r * x + sqrt(1 - (1 - x * x) / (r*r))))^2;
    f =  @(x) .5 * (Rs(x) + Rp(x));
    
    y(j) = r;
    for i = 1:M
        v = (i-1)/M * pi/2;
        
        x(i) = cos(v);
        if (v > s)
            z(j,i) = 1;
        else
            z(j,i) = f(cos(v));
        end
    end
end

F = @(x, y) min(1, max(0, ((1-y)/(1+y))^2 + ...
    (1-((1-y)/(1+y))^2) * (1 - (x - sqrt(1 - min(1, y*y)))/(1 - sqrt(1 - min(1, y*y))))^(6 + 18*exp(-13*max(0,y-1)))));

fmesh(F,[0 1 Rmin Rmax],'o')
hold on
surf(x,y,z);
hold off

% X = linspace(max(1e-8, cos_sigma),1,100);
% Y = f(X);

% ft = fittype('min(1, max(0, ((1-y)/(1+y))^2 + (1-((1-y)/(1+y))^2) * (1 - (x - sqrt(1 - min(1, y*y)))/(1 - sqrt(1 - min(1, y*y))))^(6 + 18*exp(-13*max(0,y-1)))))','independent',{'x', 'y'},'dependent',{'z'});
% opts = fitoptions(ft);
% opts.MaxIter = 100;
% opts.MaxFunEvals = 80;
% 
% sf = fit([x, y], z, ft, opts); 
% surffit = fit([x,y],z,'poly23','normalize','on');
% plot(sf,[x,y],z);

%sf = fit(X(:), Y(:), 'gauss2');
% plot(sf,X(:),Y(:),'o');

% lims = [cos_sigma 1];
% fplot(f, lims, 'r');
% hold on
% fplot(g, lims, 'g');
% hold off
end
