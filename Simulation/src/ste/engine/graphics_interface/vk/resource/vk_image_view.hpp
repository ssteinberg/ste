//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <ste.hpp>

#include <vulkan/vulkan.h>
#include <vk_image.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <vk_image_type.hpp>
#include <vk_image_view_swizzle.hpp>

#include <limits>

namespace StE {
namespace GL {

template <vk_image_type type>
class vk_image_view {
private:
	static constexpr int ctor_array_layers_multiplier = vk_image_is_cubemap<type>::value ? 6 : 1;

private:
	VkImageView view{ VK_NULL_HANDLE };
	const vk_logical_device &device;
	VkFormat format;

protected:
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t base_mip,
				  std::uint32_t mips,
				  std::uint32_t base_layer,
				  std::uint32_t layers,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle())
		: device(parent.get_creating_device()), format(format)
	{
		VkImageView view;

		VkImageSubresourceRange range;
		range.baseArrayLayer = base_layer;
		range.layerCount = layers;
		range.baseMipLevel = base_mip;
		range.levelCount = mips;
		range.aspectMask = depth_aspect ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.image = parent;
		create_info.viewType = vk_image_vk_type<type>::value;
		create_info.format = format;
		create_info.components = swizzle;
		create_info.subresourceRange = range;

		vk_result res = vkCreateImageView(device, &create_info, nullptr, &view);
		if (!res) {
			throw vk_exception(res);
		}

		this->view = view;
	}

public:
	// Non-cubemaps
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t base_mip,
				  std::uint32_t mips,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						base_mip,
						mips,
						base_layer,
						1 * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle) {}
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t mips = std::numeric_limits<std::uint32_t>::max(),
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						0,
						glm::min(parent.get_mips(), mips),
						base_layer,
						1 * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle) {}
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						0) {}

	// Arrays
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t layers,
				  std::uint32_t base_mip,
				  std::uint32_t mips,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						base_mip,
						mips,
						base_layer,
						layers * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle) {}
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t layers,
				  std::uint32_t mips = std::numeric_limits<std::uint32_t>::max(),
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						0,
						glm::min(parent.get_mips(), mips),
						base_layer,
						layers * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle) {}
	template <typename T>
	vk_image_view(const vk_image<T> &parent,
				  VkFormat format,
				  std::uint32_t layers,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<vk_image_has_arrays<type>::value>* = nullptr)
		: vk_image_view(parent,
						format,
						layers,
						0,
						depth_aspect,
						swizzle) {}

	~vk_image_view() noexcept { destroy_view(); }

	vk_image_view(vk_image_view &&) = default;
	vk_image_view& operator=(vk_image_view &&) = default;
	vk_image_view(const vk_image_view &) = delete;
	vk_image_view& operator=(const vk_image_view &) = delete;

	void destroy_view() {
		if (view != VK_NULL_HANDLE) {
			vkDestroyImageView(device, view, nullptr);
			view = VK_NULL_HANDLE;
		}
	}

	auto& get_view() const { return view; }
	auto& get_format() const { return format; }

	operator VkImageView() const { return get_view(); }
};

}
}
