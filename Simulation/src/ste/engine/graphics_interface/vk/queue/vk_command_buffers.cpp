
#include <stdafx.hpp>
#include <vk_command_buffers.hpp>
#include <vk_command_pool.hpp>

using namespace StE::GL;

void vk_command_buffers::free() {
	if (buffers.size()) {
		vkFreeCommandBuffers(device, pool, buffers.size(), reinterpret_cast<VkCommandBuffer*>(&buffers[0]));
		buffers.clear();
	}
}
