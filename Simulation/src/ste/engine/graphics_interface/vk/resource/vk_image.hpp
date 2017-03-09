//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_resource.hpp>
#include <vk_image_base.hpp>

#include <vk_logical_device.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_device_memory.hpp>

namespace StE {
namespace GL {

template <int dimensions>
class vk_image : public vk_image_base<dimensions>, public vk_resource {
public:
	using vk_image_base<dimensions>::size_type;

private:
	VkImageUsageFlags usage;
	bool sparse;

private:
	static auto vk_type() {
		switch (dimensions) {
		default:
			assert(false);
		case 1:
			return VK_IMAGE_TYPE_1D;
		case 2:
			return VK_IMAGE_TYPE_2D;
		case 3:
			return VK_IMAGE_TYPE_3D;
		}
	}

protected:
	void bind_resource_underlying_memory(const vk_device_memory &memory, std::uint64_t offset) override {
		vk_result res = vkBindImageMemory(this->device, *this, memory, offset);
		if (!res) {
			throw vk_exception(res);
		}
	}

public:
	vk_image(const vk_logical_device &device,
			 const VkFormat &format,
			 const size_type &size,
			 const VkImageUsageFlags &usage,
			 std::uint32_t mips = 1,
			 std::uint32_t layers = 1,
			 const VkImageLayout &initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
			 bool optimal_tiling = true,
			 bool sparse = false)
		: vk_image_base(device, format, size, mips, layers), usage(usage), sparse(sparse)
	{
		VkImage image;

		glm::u32vec3 extent(0);
		for (int i = 0; i < dimensions; ++i)
			extent[i] = size[i];

		auto flags = sparse ?
			VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT :
			0;
		flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VkImageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = flags;
		create_info.imageType = vk_type();
		create_info.format = format;
		create_info.extent = { extent.x, extent.y, extent.z };
		create_info.mipLevels = mips;
		create_info.arrayLayers = layers;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.tiling = optimal_tiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.usage = usage;
		create_info.initialLayout = initial_layout;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;

		vk_result res = vkCreateImage(device, &create_info, nullptr, &image);
		if (!res) {
			throw vk_exception(res);
		}

		this->image = image;
	}
	~vk_image() noexcept { destroy_image(); }

	vk_image(vk_image &&) = default;
	vk_image& operator=(vk_image &&) = default;
	vk_image(const vk_image &) = delete;
	vk_image& operator=(const vk_image &) = delete;

	void destroy_image() {
		if (this->image) {
			vkDestroyImage(this->device, *this, nullptr);
			this->image = none;
		}
	}

	VkMemoryRequirements get_memory_requirements() const override {
		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(this->device, 
									 this->image, 
									 &req);

		return req;
	}

	auto& get_usage() const{ return usage; };
	auto is_sparse() const{ return sparse; };
};

}
}
