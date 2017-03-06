//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>

namespace StE {
namespace GL {

class vk_render_pass_attachment {
private:


public:
	vk_render_pass_attachment() {}

	operator VkAttachmentDescription() const {  }
};

}
}
