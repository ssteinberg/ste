//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_image.hpp>
#include <device_image_layout.hpp>

#include <vk_command.hpp>

namespace StE {
namespace GL {

template <int dimensions, class allocation_policy>
auto image_layout_transform_barrier(const device_image<dimensions, allocation_policy> &image,
									VkAccessFlags src_access,
									VkImageLayout src_layout,
									VkAccessFlags dst_access,
									VkImageLayout dst_layout,
									bool depth = false) {
	return vk_image_memory_barrier(image.get(),
								   src_layout,
								   dst_layout,
								   src_access,
								   dst_access,
								   depth);
}

class cmd_image_layout_transform : public vk_command {
private:
	device_image_layout_transformable *image;
	VkImage image_handle;
	VkPipelineStageFlags src_stage;
	VkAccessFlags src_access;
	VkPipelineStageFlags dst_stage;
	VkAccessFlags dst_access;
	VkImageLayout dst_layout;
	bool depth;

public:
	template <int dimensions, class allocation_policy>
	cmd_image_layout_transform(device_image<dimensions, allocation_policy> &image,
							   VkPipelineStageFlags src_stage,
							   VkAccessFlags src_access,
							   VkPipelineStageFlags dst_stage,
							   VkAccessFlags dst_access,
							   VkImageLayout dst_layout,
							   bool depth = false)
		: image(&image),
		image_handle(image),
		src_stage(src_stage),
		src_access(src_access),
		dst_stage(dst_stage),
		dst_access(dst_access),
		dst_layout(dst_layout),
		depth(depth)
	{}
	virtual ~cmd_image_layout_transform() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		auto src_layout = image->layout();
		execute(command_buffer, vk_cmd_pipeline_barrier(vk_pipeline_barrier(src_stage,
																			dst_stage,
																			vk_image_memory_barrier(image_handle,
																									src_layout,
																									dst_layout,
																									src_access,
																									dst_access,
																									depth))));
		image->image_layout.layout.store(dst_layout, std::memory_order_release);
	}
};

class cmd_image_layout_transform_discard : public vk_command {
private:
	device_image_layout_transformable *image;
	VkImage image_handle;
	VkPipelineStageFlags src_stage;
	VkAccessFlags src_access;
	VkPipelineStageFlags dst_stage;
	VkAccessFlags dst_access;
	VkImageLayout dst_layout;
	bool depth;

public:
	template <int dimensions, class allocation_policy>
	cmd_image_layout_transform_discard(device_image<dimensions, allocation_policy> &image,
									   VkPipelineStageFlags src_stage,
									   VkAccessFlags src_access,
									   VkPipelineStageFlags dst_stage,
									   VkAccessFlags dst_access,
									   VkImageLayout dst_layout,
									   bool depth = false)
		: image(&image),
		image_handle(image),
		src_stage(src_stage),
		src_access(src_access),
		dst_stage(dst_stage),
		dst_access(dst_access),
		dst_layout(dst_layout),
		depth(depth)
	{}
	virtual ~cmd_image_layout_transform_discard() noexcept {}

private:
	void operator()(const vk_command_buffer &command_buffer) const override final {
		execute(command_buffer, vk_cmd_pipeline_barrier(vk_pipeline_barrier(src_stage,
																			dst_stage,
																			vk_image_memory_barrier(image_handle,
																									VK_IMAGE_LAYOUT_UNDEFINED,
																									dst_layout,
																									src_access,
																									dst_access,
																									depth))));
		image->image_layout.layout.store(dst_layout, std::memory_order_release);
	}
};

template <int dimensions, class allocation_policy>
void image_layout_transform(device_image<dimensions, allocation_policy> &image,
							VkPipelineStageFlags src_stage,
							VkAccessFlags src_access,
							VkPipelineStageFlags dst_stage,
							VkAccessFlags dst_access,
							VkImageLayout dst_layout,
							bool depth = false) {
	auto q_idx = image.owner_queue_index();
	auto& q = image.ctx.device().select_queue(q_idx);

	q->enqueue([=, &image]() {
		auto batch = ste_device_queue::thread_allocate_batch();
		auto& command_buffer = batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();
			recorder << cmd_image_layout_transform(image,
												   src_stage,
												   src_access,
												   dst_stage,
												   dst_access,
												   dst_layout,
												   depth);
		}

		ste_device_queue::submit_batch(std::move(batch), {}, {});
	});
}

template <int dimensions, class allocation_policy>
void image_layout_transform_discard(device_image<dimensions, allocation_policy> &image,
									VkPipelineStageFlags src_stage,
									VkAccessFlags src_access,
									VkPipelineStageFlags dst_stage,
									VkAccessFlags dst_access,
									VkImageLayout dst_layout,
									bool depth = false) {
	auto q_idx = image.owner_queue_index();
	auto& q = image.ctx.device().select_queue(q_idx);

	q->enqueue([=, &image]() {
		auto batch = ste_device_queue::thread_allocate_batch();
		auto& command_buffer = batch->acquire_command_buffer();
		{
			auto recorder = command_buffer.record();
			recorder << cmd_image_layout_transform_discard(image,
														   src_stage,
														   src_access,
														   dst_stage,
														   dst_access,
														   dst_layout,
														   depth);
		}

		ste_device_queue::submit_batch(std::move(batch), {}, {});
	});
}

}
}
