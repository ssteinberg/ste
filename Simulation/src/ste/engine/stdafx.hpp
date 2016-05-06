// StE
// © Shlomi Steinberg, 2015-2016

#ifndef _PCH_
#define _PCH_

#include <cstdint>

#include "gl.hpp"

#define GLM_FORCE_AVX
#define GLM_EXT_INCLUDED
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/bit.hpp>

#include <gli/format.hpp>
#include <gli/gl.hpp>
#include <gli/texture1d.hpp>
#include <gli/texture1d_array.hpp>
#include <gli/texture2d.hpp>
#include <gli/texture2d_array.hpp>
#include <gli/texture3d.hpp>
#include <gli/texture_cube.hpp>
#include <gli/texture_cube_array.hpp>

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <utility>
#include <string>
#include <iostream>

#endif
