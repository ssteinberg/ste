// StE
// © Shlomi Steinberg, 2015-2016

#ifndef _PCH_
#define _PCH_

#pragma warning push
#pragma warning(disable:869)

#include <cstdint>

#ifdef _MSC_VER
#include <windows.hpp>
#endif
#include <vulkan/vulkan.h>

#include <gl/glew.h>

#pragma warning push
#pragma warning(disable:186)
#pragma warning(disable:3280)
#pragma warning(disable:470)

#define GLM_FORCE_AVX
#define GLM_EXT_INCLUDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

#pragma warning pop


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
