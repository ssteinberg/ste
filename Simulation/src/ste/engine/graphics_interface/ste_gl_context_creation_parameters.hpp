//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_physical_device_descriptor.hpp>

#include <ste_presentation_surface_creation_parameters.hpp>

#include <optional.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

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

	bool allow_markers{ false };

	ste_presentation_surface_creation_parameters presentation_surface_parameters;
};

}
}
