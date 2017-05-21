//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <vulkan/vulkan.h>
#include <vk_rasterizer_op_descriptor.hpp>

#include <cull_mode.hpp>
#include <front_face.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class rasterizer_operation : public allow_type_decay<rasterizer_operation, vk::vk_rasterizer_op_descriptor> {
private:
	vk::vk_rasterizer_op_descriptor op;

public:
	rasterizer_operation() = default;
	rasterizer_operation(bool discard)
		: op(VK_CULL_MODE_BACK_BIT,
			 VK_FRONT_FACE_COUNTER_CLOCKWISE,
			 false,
			 .0f,
			 .0f,
			 discard)
	{}
	rasterizer_operation(cull_mode cull_mode,
						 front_face front_face)
		: op(static_cast<VkCullModeFlags>(cull_mode),
			 static_cast<VkFrontFace>(front_face),
			 false,
			 .0f,
			 .0f,
			 false)
	{}
	rasterizer_operation(cull_mode cull_mode,
						 front_face front_face,
						 bool discard)
		: op(static_cast<VkCullModeFlags>(cull_mode),
			 static_cast<VkFrontFace>(front_face),
			 false,
			 .0f,
			 .0f,
			 discard)
	{}
	rasterizer_operation(float depth_bias_const_factor,
						 float depth_bias_slope_factor)
		: op(VK_CULL_MODE_BACK_BIT,
			 VK_FRONT_FACE_COUNTER_CLOCKWISE,
			 true,
			 depth_bias_const_factor,
			 depth_bias_slope_factor,
			 false)
	{}
	rasterizer_operation(cull_mode cull_mode,
						 front_face front_face,
						 float depth_bias_const_factor,
						 float depth_bias_slope_factor,
						 bool discard = false)
		: op(static_cast<VkCullModeFlags>(cull_mode),
			 static_cast<VkFrontFace>(front_face),
			 true,
			 depth_bias_const_factor,
			 depth_bias_slope_factor,
			 discard)
	{}

	auto &get() const { return op; }
};

}
}
