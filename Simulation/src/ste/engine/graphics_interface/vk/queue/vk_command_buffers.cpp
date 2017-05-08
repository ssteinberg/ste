
#include <stdafx.hpp>
#include <vk_command_buffers.hpp>
#include <vk_command_pool.hpp>

using namespace ste::gl;

void vk::vk_command_buffers::free() {
	if (buffers.size() == 1) {
		vkFreeCommandBuffers(device.get(), pool, 1, &buffers[0].get());
		buffers.clear();
	}
	else if (buffers.size()) {
		std::vector<VkCommandBuffer> b;
		b.reserve(buffers.size());
		for (auto &e : buffers)
			b.push_back(e.get());

		vkFreeCommandBuffers(device.get(), pool, b.size(), b.data());
		buffers.clear();
	}
}
