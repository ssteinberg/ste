//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_descriptor_set_write_resource.hpp>
#include <pipeline_binding_layout_interface.hpp>

#include <buffer_view.hpp>
#include <device_buffer.hpp>
#include <device_buffer_sparse.hpp>
#include <image_view.hpp>
#include <sampler.hpp>

#include <combined_image_sampler.hpp>
#include <image.hpp>

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
	return gl::bind(array.get());
}
/**
*	@brief	Buffe binder helper. Binds a Ring buffer.
*
*	@param	ring	Ring buffer to bind
*/
template <typename T>
auto bind(const ring_buffer<T> &ring) {
	return gl::bind(ring.get());
}
/**
*	@brief	Buffe binder helper. Binds a stable vector.
*
*	@param	vec		Stable vector to bind
*/
template <typename T, std::uint64_t a, std::uint64_t b>
auto bind(const stable_vector<T, a, b> &vec) {
	return gl::bind(vec.get());
}

/**
*	@brief	Creates an image binder
*
*	@param	img		Image to bind
*/
auto inline bind(const pipeline::image &img) {
	VkImageView v = img.get_image_view().get_image_view_handle();
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, image_view_generic>(0, {
		vk::vk_descriptor_set_write_image{ v, static_cast<VkImageLayout>(img.get_layout()) }
	});
}
/**
*	@brief	Creates an image view binder
*
*	@param	array_element	First array element to bind
*	@param	images			Images to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<pipeline::image> &images) {
	assert(images.size());

	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(images.size());
	for (auto &img : images) {
		VkImageView v = img.get_image_view().get_image_view_handle();
		writes.push_back({ v, static_cast<VkImageLayout>(img.get_layout()) });
	}
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, image_view_generic>(array_element, std::move(writes));
}
/**
*	@brief	Creates an image view binder
*
*	@param	images			Images to bind
*/
auto inline bind(const std::vector<pipeline::image> &images) {
	assert(images.size());
	return gl::bind(0, images);
}

/**
*	@brief	Creates an combined_image_sampler binder
*
*	@param	cis		Combined-image-sampler to bind
*/
auto inline bind(const pipeline::combined_image_sampler& cis) {
	VkImageView v = cis.get_image_view().get_image_view_handle();
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, pipeline::combined_image_sampler>(0, {
		vk::vk_descriptor_set_write_image{ v, static_cast<VkImageLayout>(cis.get_layout()), cis.get_sampler() }
	});
}
/**
*	@brief	Creates an combined_image_sampler binder
*
*	@param	array_element	First array element to bind
*	@param	ciss				Combined-image-samplers to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<pipeline::combined_image_sampler> &ciss) {
	assert(ciss.size());

	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(ciss.size());
	for (auto &p : ciss) {
		VkImageView v = p.get_image_view().get_image_view_handle();
		writes.push_back({ v, static_cast<VkImageLayout>(p.get_layout()), p.get_sampler() });
	}
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, pipeline::combined_image_sampler>(array_element, std::move(writes));
}
/**
*	@brief	Creates an combined_image_sampler binder
*
*	@param	ciss		Combined-image-samplers to bind
*/
auto inline bind(const std::vector<pipeline::combined_image_sampler> &ciss) {
	assert(ciss.size());
	return gl::bind(0, ciss);
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
*	@param	array_element	First array element to bind
*	@param	samplers		Samplers to bind
*/
auto inline bind(std::uint32_t array_element,
				 const std::vector<const sampler*> &samplers) {
	assert(samplers.size());

	std::vector<vk::vk_descriptor_set_write_image> writes;
	writes.reserve(samplers.size());
	for (auto &p : samplers)
		writes.push_back({ p->get() });
	return pipeline_resource_binder<vk::vk_descriptor_set_write_image, sampler>(array_element, std::move(writes));
}
/**
*	@brief	Creates an sampler binder
*
*	@param	samplers	Sampelrs to bind
*/
auto inline bind(const std::vector<const sampler*> &samplers) {
	assert(samplers.size());
	return gl::bind(0, samplers);
}

}
}
