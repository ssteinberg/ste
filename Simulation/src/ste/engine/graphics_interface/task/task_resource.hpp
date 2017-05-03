//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_stage.hpp>
#include <image_layout.hpp>
#include <access_flags.hpp>

namespace ste {
namespace gl {

using task_resource_handle = std::uint64_t;

enum class task_resource_type : std::uint32_t {
	image,
	image_view,
	buffer,
	buffer_view,
	sampler,
};

struct task_resource {
	pipeline_stage stage;
	access_flags access;

	task_resource_handle handle;
	image_layout layout;
};

}
}
