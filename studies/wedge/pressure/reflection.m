%%
% Compute acoustic reflection loss from ocean bottom
%
%   R = reflection( angle, density, speed, atten, speed_shear, atten_shear ) 
%
%   R           = complex reflection coefficient for a plane wave
%   angle       = Reflection angle relative to the normal (radians).
%   density     = Ratio of bottom density to water density 
%   speed       = Ratio of compressional sound speed in the bottom to
%               the sound speed in water. 
%   atten       = Compressional wave attenuation in bottom (dB/wavelength).  
%               No compressional attenuation if this is zero.
%   speed_shear = Ratio of shear speed in the bottom to
%               the compressional sound speed in water. 
%               No shear component if this is zero.
%   atten_shear = Shear wave attenuation in bottom (dB/wavelength).  
%               No shear attenuation if this is zero.
%
function R = reflection( angle, density, speed, atten, speed_shear, atten_shear ) 

    % compute the compressional elements of water and sediment impedance
    
    Zw = 1.0 ./ cos(angle) ;
    Zb = impedance( angle, density, speed, atten ) ;
        
    % compute shear wave elements of sediment impedance
    
    if ( nargin > 4 && speed_shear >= 0.0 )
        [Zs, cosAs] = impedance( angle, density, speed_shear, atten_shear ) ;
        sinAs = sqrt( 1.0 - cosAs .* cosAs ) ;
        cos2As = 2.0 * cosAs .* cosAs - 1.0 ;
        sin2As = 2.0 * sinAs .* cosAs ;
        Zb = Zb .* cos2As .* cos2As + Zs .* sin2As .* sin2As ;
    end
        
    % compute reflection loss
    
    R = ( Zb - Zw ) ./ ( Zb + Zw ) ;
end

%%
% Compute sediment impedance.  Use for both compressional and shear waves.
%
function [Z,cosA] = impedance( angle, density, speed, atten ) 
    atten = atten / (20.0*log10(exp(1))*2*pi);  % loss tangent
    c = speed .* (( 1 - 1i.*atten )*ones(size(speed))) ;
    sinA = sin(angle) .* c ;				% Snell's Law
    cosA = sqrt( 1.0 - sinA.*sinA ) ;
    Z = c .* density ./ cosA ;
end
