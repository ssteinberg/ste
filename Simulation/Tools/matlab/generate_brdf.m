function [ brdf, btdf, X, Y ] = generate_brdf(is_conductor, D, PDF, CDF, CDFinv, Lambda, P1, C1, C1inv, N, Samples, v, r, roughness)

    brdf = zeros(N, N);
    btdf = 0;
    
    function [h2] = sample(w,h,U)
        if (w(3) == 0)
            h2 = h;
            return;
        end
        if (w(3) < 0)
            h2 = C1inv(max(0, C1(h) - U));
            return;
        end
        
        theta = spherical_from_vec(w);
        l = Lambda(abs(theta));
        
        G1dist = C1(-h) ^ l;
        
        if (U >= 1 - G1dist)
            h2 = inf(1);
        else
            h2 = C1inv(min(1, C1(h) / (1-U)^(1/l)));
        end
    end
    
    function march_ray(w,h,e,inmedium,d)
        h = sample(w,h,rand());
        assert(~isnan(h));

        too_many_jumps = d>50;
        if (isinf(h) || too_many_jumps)
            if (h < 0 || too_many_jumps)
                btdf = btdf + e;
            else
                [i, j] = spherical_from_vec(w);
                i = floor(i / (pi/2) * (N - 1e-60)) + 1;
                assert(i>0 && i<=N);
                if (i>0)
                    j = floor(abs(j) / pi * (N - 1e-60)) + 1;
                    assert(j>0 && j<=N);
                    brdf(i,j) = brdf(i,j) + e;
                end
            end
            return;
        end
        
%         if (e < 1e-14)
%             btdf = btdf + e;
%             return;
%         end

%             theta_m = (k-.5)/Samples * pi - pi/2;

        theta_m = CDFinv(rand());
        phi_m = rand()*2*pi;

%             theta_m_top = k/Samples * pi - pi/2;
%             theta_m_bottom = (k-1)/Samples * pi - pi/2;
%             weight = abs(CDF(abs(theta_m_top)) - CDF(abs(theta_m_bottom)));

        m = [sin(theta_m) * cos(phi_m), sin(theta_m) * sin(phi_m), cos(theta_m)];

        if (dot(-w,m)<0)
            m = -m;
        end
            
        assert(dot(-w,m)>=0);

        ior_ratio = r;
        if (inmedium == 1)
            ior_ratio = 1/r;
        end
            
        f = fresnelv(-w, m, ior_ratio);
        weighto = f;
        weightt = 1 - f;

        wo = reflect(w, m);

        march_ray(wo,h,e*weighto,inmedium,d+1);
        if (is_conductor ~= 1)
            [wt, is_tir] = refract(w, m, ior_ratio);
            if (is_tir ~= 1)
                march_ray(wt/norm(wt),h,e*weightt,~inmedium,d+1);
            end
        end
%             end
    end
    
    for ii = 1:N
        X(ii) = (ii-1) / (N-1) * pi/2;
        Y(ii) = (ii-1) / (N-1) * pi;
    end

    w0 = -[sin(v), 0, cos(v)];
    h0 = Inf(1);

    for k = 1:Samples
        march_ray(w0,h0,1,0,1);

        if (mod(k, 10000) == 0)
            fprintf('%d\n', k);
        end
    end

end

