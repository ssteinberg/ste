//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_buffer.hpp>
#include <device_resource.hpp>
#include <device_resource_allocation_policy.hpp>

namespace StE {
namespace GL {

template <typename T, class allocation_policy = device_resource_allocation_policy_device>
using device_buffer = device_resource<vk_buffer<T>, allocation_policy>;

}
}
