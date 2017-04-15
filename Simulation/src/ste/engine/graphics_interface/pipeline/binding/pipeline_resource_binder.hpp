//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_layout_collection.hpp>
#include <vk_descriptor_set_write_resource.hpp>

#include <buffer_view.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <texture.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <array.hpp>
#include <ring_buffer.hpp>
#include <stable_vector.hpp>

#include <vector>

namespace StE {
namespace GL {

template <
	typename WriteType,
	typename T
>
class pipeline_resource_binder {
public:
	using UnderlyingType = T;

private:
	std::uint32_t array_element;
	std::vector<WriteType> writes;

public:
	pipeline_resource_binder(std::uint32_t array_element,
							 std::vector<WriteType> &&writes)
		: array_element(array_element),
		writes(std::move(writes))
	{
	}
	~pipeline_resource_binder() noexcept {}

	pipeline_resource_binder(pipeline_resource_binder&&) = default;
	pipeline_resource_binder &operator=(pipeline_resource_binder&&) = default;

	vk_descriptor_set_write_resource writer(const pipeline_binding_set_layout_binding *binding) const {
		return vk_descriptor_set_write_resource(*binding->binding,
												binding->binding->bind_idx,
												array_element,
												writes);
	}
};

/**
*	@brief	Creates a buffer binder
*
*	@param	buffer	Buffer to bind
*/
template <typename T, class a>
auto bind(const device_buffer<T, a> &buffer) {
	return pipeline_resource_binder<vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer) });
}
/**
*	@brief	Creates a buffer binder
*
*	@param	buffer	Buffer to bind
*	@param	offset	Buffer offset
*	@param	range	Buffer range
*/
template <typename T, class a>
auto bind(const device_buffer<T, a> &buffer,
		  std::uint64_t offset,
		  std::uint64_t range) {
	return pipeline_resource_binder<vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer, offset, range) });
}
/**
*	@brief	Creates a buffer binder
*
*	@param	buffer	Buffer to bind
*	@param	offset	Buffer offset
*	@param	range	Buffer range
*/
template <typename T, std::uint64_t s, class a>
auto bind(const device_buffer_sparse<T, s, a> &buffer,
		  std::uint64_t offset,
		  std::uint64_t range) {
	return pipeline_resource_binder<vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer, offset, range) });
}

/**
*	@brief	Buffe binder helper. Binds an array.
*
*	@param	array	Array to bind
*/
template <typename T>
auto bind(const array<T> &array) {
	return bind(array.get());
}
/**
*	@brief	Buffe binder helper. Binds a Ring buffer.
*
*	@param	ring	Ring buffer to bind
*/
template <typename T>
auto bind(const ring_buffer<T> &ring) {
	return bind(ring.get());
}
/**
*	@brief	Buffe binder helper. Binds a stable vector.
*
*	@param	vec		Stable vector to bind
*/
template <typename T, std::uint64_t a, std::uint64_t b>
auto bind(const stable_vector<T, a, b> &vec) {
	return bind(vec.get());
}

/**
*	@brief	Creates an image view binder
*
*	@param	image	Image view to bind
*	@param	layout	Image layout
*/
auto inline bind(const image_view_generic &image,
				 VkImageLayout layout) {
	return pipeline_resource_binder<vk_descriptor_set_write_image, image_view_generic>(0, {
		vk_descriptor_set_write_image{ image.get_image_view_handle(), layout }
	});
}
/**
*	@brief	Creates an image view binder
*
*	@param	images	Pairs of image views and layouts to bind
*/
auto inline bind(const std::vector<std::pair<const image_view_generic*, VkImageLayout>> &images) {
	return bind(0, images);
}
/**
*	@brief	Creates an image view binder
*
*	@param	array_element	First array element to bind
*	@param	images	Pairs of image views and layouts to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<std::pair<const image_view_generic*, VkImageLayout>> &images) {
	std::vector<vk_descriptor_set_write_image> writes;
	writes.reserve(images.size());
	for (auto &p : images)
		writes.push_back({ p.first->get_image_view_handle(), p.second });
	return pipeline_resource_binder<vk_descriptor_set_write_image, image_view_generic>(array_element, std::move(writes));
}

/**
*	@brief	Creates an texture binder
*
*	@param	tex		Texture to bind
*	@param	layout	Image layout
*/
auto inline bind(const texture_generic &tex,
				 VkImageLayout layout) {
	return pipeline_resource_binder<vk_descriptor_set_write_image, texture_generic>(0, {
		vk_descriptor_set_write_image{ tex.get_image_view_handle(), layout, tex.get_sampler() }
	});
}
/**
*	@brief	Creates an texture binder
*
*	@param	textures	Pairs of textures and layouts to bind
*/
auto inline bind(const std::vector<std::pair<const texture_generic*, VkImageLayout>> &textures) {
	return bind(0, textures);
}
/**
*	@brief	Creates an texture binder
*
*	@param	array_element	First array element to bind
*	@param	textures	Pairs of textures and layouts to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<std::pair<const texture_generic*, VkImageLayout>> &textures) {
	std::vector<vk_descriptor_set_write_image> writes;
	writes.reserve(textures.size());
	for (auto &p : textures)
		writes.push_back({ p.first->get_image_view_handle(), p.second, p.first->get_sampler() });
	return pipeline_resource_binder<vk_descriptor_set_write_image, texture_generic>(array_element, std::move(writes));
}

/**
*	@brief	Creates an sampler binder
*
*	@param	sam		Sampler to bind
*/
auto inline bind(const sampler &sam) {
	return pipeline_resource_binder<vk_descriptor_set_write_image, sampler>(0, {
		vk_descriptor_set_write_image(sam.get())
	});
}
/**
*	@brief	Creates an sampler binder
*
*	@param	samplers	Sampelrs to bind
*/
auto inline bind(const std::vector<const sampler*> &samplers) {
	return bind(0, samplers);
}
/**
*	@brief	Creates an sampler binder
*
*	@param	array_element	First array element to bind
*	@param	samplers		Samplers to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<const sampler*> &samplers) {
	std::vector<vk_descriptor_set_write_image> writes;
	writes.reserve(samplers.size());
	for (auto &p : samplers)
		writes.push_back({ p->get() });
	return pipeline_resource_binder<vk_descriptor_set_write_image, sampler>(array_element, std::move(writes));
}

}
}
