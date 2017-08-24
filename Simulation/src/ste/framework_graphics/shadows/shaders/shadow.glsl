
layout(set=2, binding=10) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(set=2, binding=11) uniform samplerCubeArray shadow_maps;
layout(set=2, binding=12) uniform sampler2DArrayShadow directional_shadow_depth_maps;
layout(set=2, binding=13) uniform sampler2DArray directional_shadow_maps;

#include <shadow_query_spherical.glsl>
#include <shadow_query_directional.glsl>
