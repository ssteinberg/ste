//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_descriptor_set_write_resource.hpp>
#include <pipeline_binding_layout_interface.hpp>

#include <buffer_view.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <texture.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <image_layout.hpp>

#include <array.hpp>
#include <ring_buffer.hpp>
#include <stable_vector.hpp>

#include <vector>

namespace ste {
namespace gl {

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

	vk::vk_descriptor_set_write_resource writer(const pipeline_binding_layout_interface *binding) const {
		return vk::vk_descriptor_set_write_resource(binding->vk_descriptor_type(),
													binding->bind_idx(),
													array_element,
													writes);
	}

	const auto &get_writes() const { return writes; }
	auto get_array_element() const { return array_element; }
};

/**
*	@brief	Creates a buffer binder
*
*	@param	buffer	Buffer to bind
*/
template <typename T, class a>
auto bind(const device_buffer<T, a> &buffer) {
	return pipeline_resource_binder<vk::vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer) });
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
	return pipeline_resource_binder<vk::vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer, offset, range) });
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
	return pipeline_resource_binder<vk::vk_descriptor_set_write_buffer, T>(0, { buffer_view(buffer, offset, range) });
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
				 image_layout layout) {
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, image_view_generic>(0, {
		vk::vk_descriptor_set_write_image{ image.get_image_view_handle(), static_cast<VkImageLayout>(layout) }
	});
}
/**
*	@brief	Creates an image view binder
*
*	@param	images	Pairs of image views and layouts to bind
*/
auto inline bind(const std::vector<std::pair<const image_view_generic*, image_layout>> &images) {
	return bind(0, images);
}
/**
*	@brief	Creates an image view binder
*
*	@param	array_element	First array element to bind
*	@param	images	Pairs of image views and layouts to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<std::pair<const image_view_generic*, image_layout>> &images) {
	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(images.size());
	for (auto &p : images)
		writes.push_back({ p.first->get_image_view_handle(), static_cast<VkImageLayout>(p.second) });
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, image_view_generic>(array_element, std::move(writes));
}

/**
*	@brief	Creates an texture binder
*
*	@param	tex		Texture to bind
*/
auto inline bind(const texture_generic &tex) {
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, texture_generic>(0, {
		vk::vk_descriptor_set_write_image{ tex.get_image_view_handle(), static_cast<VkImageLayout>(tex.get_layout()), tex.get_sampler() }
	});
}
/**
*	@brief	Creates an texture binder
*
*	@param	textures	Pairs of textures and layouts to bind
*/
auto inline bind(const std::vector<const texture_generic*> &textures) {
	return bind(0, textures);
}
/**
*	@brief	Creates an texture binder
*
*	@param	array_element	First array element to bind
*	@param	textures	Pairs of textures and layouts to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<const texture_generic*> &textures) {
	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(textures.size());
	for (auto &p : textures)
		writes.push_back({ p->get_image_view_handle(), static_cast<VkImageLayout>(p->get_layout()), p->get_sampler() });
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, texture_generic>(array_element, std::move(writes));
}

/**
*	@brief	Creates an sampler binder
*
*	@param	sam		Sampler to bind
*/
auto inline bind(const sampler &sam) {
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, sampler>(0, {
		vk::vk_descriptor_set_write_image(sam.get())
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
	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(samplers.size());
	for (auto &p : samplers)
		writes.push_back({ p->get() });
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, sampler>(array_element, std::move(writes));
}

}
}
