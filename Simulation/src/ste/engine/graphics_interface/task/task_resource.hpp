//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_resource_handle.hpp>
#include <pipeline_stage.hpp>
#include <image_layout.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

enum class task_resource_type : std::uint32_t {
	image,
	buffer,
};

struct task_resource {
	task_resource_type type;

	pipeline_stage stage;
	access_flags access;

	device_resource_handle handle;
	image_layout layout{ image_layout::undefined };
};

}
}
