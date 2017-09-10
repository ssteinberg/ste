//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vk_physical_device_descriptor.hpp>

#include <optional.hpp>
#include <lib/vector.hpp>

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

	lib::vector<const char*> additional_instance_extensions;
	lib::vector<const char*> additional_instance_layers;
};

struct ste_gl_device_creation_parameters {
	vk::vk_physical_device_descriptor physical_device;
	VkPhysicalDeviceFeatures requested_device_features{ 0 };
	lib::vector<const char*> additional_device_extensions;

	optional<std::uint32_t> simultaneous_presentation_frames;
	optional<ste_presentation_device_vsync> vsync;

	bool allow_markers{ false };
};

}
}
