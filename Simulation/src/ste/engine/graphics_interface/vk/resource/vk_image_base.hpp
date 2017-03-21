//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <vulkan/vulkan.h>
#include <vk_image_type.hpp>
#include <vk_image_type_traits.hpp>
#include <vk_logical_device.hpp>

#include <optional.hpp>
#include <allow_class_decay.hpp>

namespace StE {
namespace GL {

template <int dims>
class vk_image_base : public allow_class_decay<vk_image_base<dims>, VkImage> {
public:
	static constexpr int dimensions = dims;
	using size_type = typename vk_image_extent_type<dimensions>::type;

protected:
	const vk_logical_device &device;
	optional<VkImage> image;

private:
	VkFormat format;
	size_type size;
	std::uint32_t mips;
	std::uint32_t layers;

public:
	vk_image_base(const vk_logical_device &device,
				  const VkImage &image,
				  const VkFormat &format,
				  const size_type &size,
				  std::uint32_t mips,
				  std::uint32_t layers)
		: device(device), image(image), format(format), size(size), mips(mips), layers(layers)
	{
		assert(mips > 0 && "Non-positive mipmap level count");
		assert(layers > 0 && "Non-positive array layers count");
	}
	vk_image_base(const vk_logical_device &device,
				  const VkFormat &format,
				  const size_type &size,
				  std::uint32_t mips,
				  std::uint32_t layers)
		: device(device), format(format), size(size), mips(mips), layers(layers)
	{
		assert(mips > 0 && "Non-positive mipmap level count");
		assert(layers > 0 && "Non-positive array layers count");
	}
	virtual ~vk_image_base() noexcept {}

	vk_image_base(vk_image_base &&) = default;
	vk_image_base& operator=(vk_image_base &&) = default;
	vk_image_base(const vk_image_base &) = delete;
	vk_image_base& operator=(const vk_image_base &) = delete;

	auto get_image_subresource_layout(std::uint32_t mip,
									  std::uint32_t layer = 0,
									  bool color = true) {
		VkImageSubresource subresource = {};
		subresource.aspectMask = color ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
		subresource.mipLevel = mip;
		subresource.arrayLayer = layer;

		VkSubresourceLayout layout;
		vkGetImageSubresourceLayout(device, *this, &subresource, &layout);

		return layout;
	}

	auto& get_creating_device() const { return device; }
	auto& get() const { return image.get(); }

	auto& get_format() const { return format; };
	auto& get_size() const { return size; };
	auto& get_mips() const { return mips; };
	auto& get_layers() const { return layers; };
};

}
}
