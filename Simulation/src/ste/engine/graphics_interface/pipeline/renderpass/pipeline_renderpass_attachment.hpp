//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <image_view.hpp>

#include <vk_blend_op_descriptor.hpp>

namespace StE {
namespace GL {

class pipeline_renderpass_attachment {
	const image_view_generic *image;

	vk_blend_op_descriptor blend_op;
};

}
}
