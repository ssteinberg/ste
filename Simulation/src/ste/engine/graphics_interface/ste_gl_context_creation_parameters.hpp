//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_physical_device_descriptor.hpp>

#include <optional.hpp>
#include <vector>

namespace ste {
namespace gl {

enum class ste_presentation_device_vsync {
	immediate,
	fifo,
	mailbox,
};

struct ste_gl_context_creation_parameters {
	const char* client_name;
	unsigned client_version;

	optional<bool> debug_context;

	std::vector<const char*> additional_instance_extensions;
	std::vector<const char*> additional_instance_layers;
};

struct ste_gl_device_creation_parameters {
	vk::vk_physical_device_descriptor physical_device;
	VkPhysicalDeviceFeatures requested_device_features{ 0 };
	std::vector<const char*> additional_device_extensions;

	optional<ste_presentation_device_vsync> vsync;
};

}
}
