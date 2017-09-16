//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <format.hpp>
#include <presentation_surface_transform.hpp>
#include <presentation_surface_composite_alpha.hpp>

#include <optional.hpp>

namespace ste {
namespace gl {

enum class ste_presentation_device_vsync {
	immediate,
	fifo,
	mailbox,
};

struct ste_presentation_surface_creation_parameters {
	optional<std::uint32_t> simultaneous_presentation_frames;
	optional<ste_presentation_device_vsync> vsync;

	optional<format> required_format;
	optional<presentation_surface_transform> transform;
	optional<presentation_surface_composite_alpha> composite_alpha;
};

}
}