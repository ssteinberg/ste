//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_image.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy = device_resource_allocation_policy_device>
using device_image = device_resource<vk_image<dimensions>, allocation_policy>;

}
}
