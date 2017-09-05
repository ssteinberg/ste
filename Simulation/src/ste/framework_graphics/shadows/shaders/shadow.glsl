
layout(set=2, binding=23) uniform samplerCubeArrayShadow shadow_depth_maps;
layout(set=2, binding=24) uniform samplerCubeArray shadow_maps;
layout(set=2, binding=25) uniform sampler2DArrayShadow directional_shadow_depth_maps;
layout(set=2, binding=26) uniform sampler2DArray directional_shadow_maps;

#include <shadow_query_spherical.glsl>
#include <shadow_query_directional.glsl>
