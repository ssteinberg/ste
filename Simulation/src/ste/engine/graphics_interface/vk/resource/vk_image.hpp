//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_resource.hpp>
#include <image_initial_layout.hpp>

#include <vk_logical_device.hpp>
#include <vk_ext_debug_marker.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>
#include <vk_device_memory.hpp>

#include <format.hpp>
#include <vk_host_allocator.hpp>
#include <format_rtti.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace vk {

static constexpr VkImageCreateFlags vk_image_default_flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

template <typename host_allocator = vk_host_allocator<>>
class vk_image : public vk_resource<host_allocator>, public allow_type_decay<vk_image<host_allocator>, VkImage> {
protected:
	using extent_type = glm::u32vec3;

	alias<const vk_logical_device<host_allocator>> device;
	optional<VkImage> image;

	VkFormat image_format;
	extent_type extent;
	std::uint32_t mips;
	std::uint32_t layers;

	VkImageUsageFlags usage;
	bool sparse;

private:
	static auto vk_type(int dimensions) {
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
	void bind_resource_underlying_memory(const vk_device_memory<host_allocator> &memory, std::uint64_t offset) override {
		const vk_result res = vkBindImageMemory(this->device.get(), *this, memory, offset);
		if (!res) {
			throw vk_exception(res);
		}
	}

	vk_image(const vk_logical_device<host_allocator> &device,
			 const char *name,
			 VkImage image,
			 const VkFormat &image_format,
			 const extent_type &extent,
			 const VkImageUsageFlags &usage,
			 std::uint32_t mips = 1,
			 std::uint32_t layers = 1,
			 bool sparse = false)
		: device(device), image(image),
		image_format(image_format), extent(extent), mips(mips), layers(layers),
		usage(usage), sparse(sparse)
	{
		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										image,
										VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
										name);
	}

public:
	vk_image(const vk_logical_device<host_allocator> &device,
			 const char *name,
			 const image_initial_layout &layout,
			 const VkFormat &image_format,
			 int dimensions,
			 const extent_type &extent,
			 const VkImageUsageFlags &usage,
			 std::uint32_t mips = 1,
			 std::uint32_t layers = 1,
			 bool supports_cube_views = false,
			 bool optimal_tiling = true,
			 bool sparse = false)
		: device(device), image_format(image_format), extent(extent), mips(mips), layers(layers), 
		usage(usage), sparse(sparse)
	{
		assert(mips > 0 && "Non-positive mipmap level count");
		assert(layers > 0 && "Non-positive array layers count");

		VkImage image;

		auto flags = sparse ?
			VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT :
			0;
		if (supports_cube_views)
			flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		flags |= vk_image_default_flags;

		VkImageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = flags;
		create_info.imageType = vk_type(dimensions);
		create_info.format = image_format;
		create_info.extent = { extent.x, extent.y, extent.z };
		create_info.mipLevels = mips;
		create_info.arrayLayers = layers;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.tiling = optimal_tiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.usage = usage;
		create_info.initialLayout = layout == image_initial_layout::preinitialized ?
			VK_IMAGE_LAYOUT_PREINITIALIZED :
			VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;

		const vk_result res = vkCreateImage(device.get(), &create_info, &host_allocator::allocation_callbacks(), &image);
		if (!res) {
			throw vk_exception(res);
		}

		// Set object debug marker
		vk_debug_marker_set_object_name(device,
										image,
										VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
										name);

		this->image = image;
	}
	virtual ~vk_image() noexcept { destroy_image(); }

	vk_image(vk_image &&) = default;
	vk_image& operator=(vk_image &&o) noexcept {
		destroy_image();

		image = std::move(o.image);
		device = std::move(o.device);

		image_format = o.image_format;
		extent = o.extent;
		mips = o.mips;
		layers = o.layers;

		usage = o.usage;
		sparse = o.sparse;

		return *this;
	}
	vk_image(const vk_image &) = delete;
	vk_image& operator=(const vk_image &) = delete;

	void destroy_image() {
		if (this->image) {
			vkDestroyImage(this->device.get(), *this, &host_allocator::allocation_callbacks());
			this->image = none;
		}
	}

	auto get_image_subresource_layout(std::uint32_t mip,
									  std::uint32_t layer = 0) const {
		VkImageSubresource subresource = {};
		subresource.aspectMask = static_cast<VkImageAspectFlags>(format_aspect(static_cast<format>(get_format())));
		subresource.mipLevel = mip;
		subresource.arrayLayer = layer;

		VkSubresourceLayout layout;
		vkGetImageSubresourceLayout(device.get(), *this, &subresource, &layout);

		return layout;
	}

	VkMemoryRequirements get_memory_requirements() const override {
		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(this->device.get(),
									 *this,
									 &req);

		return req;
	}

	auto& get_usage() const { return usage; }
	auto is_sparse() const { return sparse; }

	auto& get_creating_device() const { return device.get(); }
	auto& get() const { return image.get(); }

	VkFormat get_format() const { return image_format; }
	auto& get_extent() const { return extent; }
	auto& get_mips() const { return mips; }
	auto& get_layers() const { return layers; }
};

}

}
}
