//  StE
// © Shlomi Steinberg 2015-2017

#ifndef _PCH_
#define _PCH_

#include <cstdint>
#include <cstddef>


#ifdef _MSC_VER
#include <windows.hpp>
#endif
#include <vulkan/vulkan.h>

#define GLM_FORCE_AVX
#define GLM_EXT_INCLUDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/bit.hpp>
#include <glm/gtc/constants.hpp>


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
