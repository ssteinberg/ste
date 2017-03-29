//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <vk_sampler.hpp>
#include <device_image_base.hpp>
#include <device_buffer_base.hpp>

namespace StE {
namespace GL {

class task_binding {
private:
	std::uint32_t binding_index;
	std::uint32_t count;
	VkDescriptorType type;

	VkAccessFlags access;
	VkPipelineStageFlags stage;

protected:
	task_binding(std::uint32_t binding_index,
				 VkDescriptorType binding_type,
				 VkAccessFlags resource_access,
				 VkPipelineStageFlags pipeline_stage,
				 std::uint32_t count)
		: binding_index(binding_index),
		count(count),
		type(binding_type),
		access(resource_access),
		stage(pipeline_stage)
	{}

public:
	virtual ~task_binding() noexcept {}

	task_binding(task_binding&&) = default;
	task_binding &operator=(task_binding&&) = default;
	task_binding(const task_binding&) = default;
	task_binding &operator=(const task_binding&) = default;

	auto& get_binding_index() const { return binding_index; }
	auto& get_count() const { return count; }
	auto& get_type() const { return type; }
};

class task_image_binding : public task_binding {
public:
	struct image_binding {
		const device_image_base *image{ nullptr };
		VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageView view{ VK_NULL_HANDLE };
		const vk_sampler *sampler{ nullptr };
	};

private:
	std::vector<image_binding> image_bindings;

public:
	task_image_binding(std::uint32_t binding_index,
					   VkDescriptorType binding_type,
					   VkAccessFlags resource_access,
					   VkPipelineStageFlags pipeline_stage,
					   const device_image_base *image,
					   VkImageLayout image_layout,
					   const VkImageView &view,
					   const vk_sampler *sampler = nullptr)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   1),
		image_bindings({ { image, image_layout, view, sampler } })
	{}
	task_image_binding(std::uint32_t binding_index,
					   VkDescriptorType binding_type,
					   VkAccessFlags resource_access,
					   VkPipelineStageFlags pipeline_stage,
					   const std::vector<image_binding> &bindings)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   bindings.size()),
		image_bindings(bindings)
	{}
	task_image_binding(std::uint32_t binding_index,
					   VkDescriptorType binding_type,
					   VkAccessFlags resource_access,
					   VkPipelineStageFlags pipeline_stage,
					   const vk_sampler *sampler)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   1),
		image_bindings({ { nullptr, VK_IMAGE_LAYOUT_UNDEFINED, VK_NULL_HANDLE, sampler } })
	{}
	virtual ~task_image_binding() noexcept {}

	task_image_binding(task_image_binding&&) = default;
	task_image_binding &operator=(task_image_binding&&) = default;
	task_image_binding(const task_image_binding&) = default;
	task_image_binding &operator=(const task_image_binding&) = default;

	auto& get_image_bindings() const { return image_bindings; }
};

class task_buffer_binding : public task_binding {
public:
	struct buffer_binding {
		const device_buffer_base *buffer;
		std::uint64_t offset_bytes{ 0 };
		std::uint64_t range_bytes{ VK_WHOLE_SIZE };
	};

private:
	std::vector<buffer_binding> buffer_bindings;

public:
	task_buffer_binding(std::uint32_t binding_index,
						VkDescriptorType binding_type,
						VkAccessFlags resource_access,
						VkPipelineStageFlags pipeline_stage,
						const device_buffer_base *buffer,
						std::uint64_t offset,
						std::uint64_t range)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   1),
		buffer_bindings({ buffer_binding{ buffer,
						offset * buffer->get_element_size_bytes(),
						range * buffer->get_element_size_bytes() } })
	{}
	task_buffer_binding(std::uint32_t binding_index,
						VkDescriptorType binding_type,
						VkAccessFlags resource_access,
						VkPipelineStageFlags pipeline_stage,
						const device_buffer_base *buffer,
						std::uint64_t offset = 0)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   1),
		buffer_bindings({ buffer_binding{ buffer,
						offset * buffer->get_element_size_bytes() } })
	{}
	task_buffer_binding(std::uint32_t binding_index,
						VkDescriptorType binding_type,
						VkAccessFlags resource_access,
						VkPipelineStageFlags pipeline_stage,
						const std::vector<buffer_binding> &buffer_bindings)
		: task_binding(binding_index,
					   binding_type,
					   resource_access,
					   pipeline_stage,
					   buffer_bindings.size()),
		buffer_bindings(buffer_bindings)
	{}
	virtual ~task_buffer_binding() noexcept {}

	task_buffer_binding(task_buffer_binding&&) = default;
	task_buffer_binding &operator=(task_buffer_binding&&) = default;
	task_buffer_binding(const task_buffer_binding&) = default;
	task_buffer_binding &operator=(const task_buffer_binding&) = default;

	auto& get_buffer_bindings() const { return buffer_bindings; };
};

}
}
