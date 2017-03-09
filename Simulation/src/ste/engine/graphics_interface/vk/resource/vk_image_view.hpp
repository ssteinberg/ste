//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>

#include <vulkan/vulkan.h>
#include <vk_image_base.hpp>
#include <vk_result.hpp>
#include <vk_exception.hpp>

#include <optional.hpp>

#include <vk_image_type.hpp>
#include <vk_image_type_traits.hpp>
#include <vk_image_view_swizzle.hpp>

#include <limits>

namespace StE {
namespace GL {

template <vk_image_type type>
class vk_image_view {
private:
	static constexpr int ctor_array_layers_multiplier = vk_image_is_cubemap<type>::value ? 6 : 1;

	struct ctor {};

public:
	static constexpr int all_mip_levels = std::numeric_limits<std::uint32_t>::max();

private:
	optional<VkImageView> view;
	const vk_logical_device &device;
	VkFormat format;

protected:
	template <int dimensions>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t base_mip,
				  std::uint32_t mips,
				  std::uint32_t base_layer,
				  std::uint32_t layers,
				  bool depth_aspect,
				  const vk_image_view_swizzle &swizzle,
				  const ctor&)
		: device(parent.get_creating_device()), format(format)
	{
		VkImageView view;

		VkImageSubresourceRange range = {};
		range.baseArrayLayer = base_layer;
		range.layerCount = layers;
		range.baseMipLevel = base_mip;
		range.levelCount = mips;
		range.aspectMask = depth_aspect ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo create_info = {};
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
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param base_mip		View base mipmap level
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t base_mip,
				  std::uint32_t mips = all_mip_levels,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						base_mip,
						glm::min(parent.get_mips() - base_mip, mips),
						base_layer * ctor_array_layers_multiplier,
						1 * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle,
						ctor()) {}
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t mips = all_mip_levels,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						base_layer,
						0,
						glm::min(parent.get_mips(), mips),
						depth_aspect,
						swizzle) {}
	/**
	*	@brief	Ctor for non-array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<!sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						0,
						all_mip_levels,
						depth_aspect,
						swizzle) {}

	/**
	*	@brief	Ctor for array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param base_mip		View base mipmap level
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param mips			View mipmap levels. Use all_mip_levels for remaining levels.
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t base_mip,
				  std::uint32_t layers,
				  std::uint32_t mips = all_mip_levels,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						base_mip,
						glm::min(parent.get_mips() - base_mip, mips),
						base_layer * ctor_array_layers_multiplier,
						layers * ctor_array_layers_multiplier,
						depth_aspect,
						swizzle,
						ctor()) {}
	/**
	*	@brief	Ctor for array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param base_layer	View base array layer. (Internally multiplied by six for cubemaps)
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t base_layer,
				  std::uint32_t layers,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						base_layer,
						0,
						layers,
						all_mip_levels,
						depth_aspect,
						swizzle) {}
	/**
	*	@brief	Ctor for array image view.
	*
	*	@param parent		Parent image object
	*	@param format		View format
	*	@param layers		View array layers. (Internally multiplied by six for cubemaps)
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions, bool sfinae = vk_image_has_arrays<type>::value>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  VkFormat format,
				  std::uint32_t layers,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle(),
				  std::enable_if_t<sfinae>* = nullptr)
		: vk_image_view(parent,
						format,
						0,
						layers,
						depth_aspect,
						swizzle) {}

	/**
	*	@brief	Ctor for image view. Creates a view of the entire mipmap chain and array layers, with identical format to parent.
	*
	*	@param parent		Parent image object
	*	@param depth_aspect	True for depth parent images, false for color images.
	*	@param swizzle		View component swizzling
	*/
	template <int dimensions>
	vk_image_view(const vk_image_base<dimensions> &parent,
				  bool depth_aspect = false,
				  const vk_image_view_swizzle &swizzle = vk_image_view_swizzle())
		: vk_image_view(parent,
						parent.get_format(),
						0,
						parent.get_mips(),
						0,
						parent.get_layers(),
						depth_aspect,
						swizzle,
						ctor()) {}

	~vk_image_view() noexcept { destroy_view(); }

	vk_image_view(vk_image_view &&) = default;
	vk_image_view& operator=(vk_image_view &&) = default;
	vk_image_view(const vk_image_view &) = delete;
	vk_image_view& operator=(const vk_image_view &) = delete;

	void destroy_view() {
		if (view) {
			vkDestroyImageView(device, *this, nullptr);
			view = none;
		}
	}

	auto& get_view() const { return view.get(); }
	auto& get_format() const { return format; }

	operator VkImageView() const { return get_view(); }
};

}
}
