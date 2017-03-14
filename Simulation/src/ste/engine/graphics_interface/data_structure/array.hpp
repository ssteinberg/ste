//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource.hpp>

#include <vk_fence.hpp>
#include <device_buffer.hpp>
#include <device_resource_allocation_policy.hpp>

#include <vk_command_recorder.hpp>
#include <vk_cmd_copy_buffer.hpp>

#include <vector>

namespace StE {
namespace GL {

template <typename T>
class array : ste_resource_deferred_create_trait {
private:
	using buffer_t = device_buffer<T, device_resource_allocation_policy_device>;
	using staging_buffer_t = device_buffer<T, device_resource_allocation_policy_host_visible_coherent>;

	static constexpr VkBufferUsageFlags buffer_usage_additional_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

private:
	const ste_context &ctx;
	buffer_t buffer;

private:
	void copy_initial_data(const std::vector<T> &data) {
		auto copy_count = std::min<std::uint64_t>(data.size(), size());

		staging_buffer_t staging_buffer(ctx, copy_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		{
			// Copy to staging
			auto ptr = staging_buffer.get_underlying_memory().template mmap<T>(0, copy_count);
			memcpy(ptr->get_mapped_ptr(), data.data(), copy_count * sizeof(T));
		}

		// Enqueue on a transfer queue
		auto future = ctx.device().enqueue(GL::make_queue_selector(GL::ste_queue_type::data_transfer_queue), [&]() {
			auto command_buffer = GL::ste_device_queue::thread_command_pool_transient().allocate_buffers(1);
			GL::vk_fence fence(ctx.device().logical_device());

			// Record and submit a one-time command buffer
			{
				GL::vk_command_recorder recorder(command_buffer[0], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				// Copy to live buffer
				recorder << GL::vk_cmd_copy_buffer(staging_buffer.get(), buffer.get());
			}
			GL::ste_device_queue::thread_queue().submit(&command_buffer[0], &fence);

			fence.wait_idle();
		});
		
		// Wait for completion
		future.wait();
	}

public:
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const VkBufferUsageFlags &usage)
		: ctx(ctx),
		buffer(ctx,
			   count,
			   usage | buffer_usage_additional_flags)
	{}
	array(const ste_context &ctx,
		  const std::vector<T> &initial_data,
		  const VkBufferUsageFlags &usage)
		: ctx(ctx),
		buffer(ctx,
			   initial_data.size(),
			   usage | buffer_usage_additional_flags)
	{
		// Copy initial static data
		copy_initial_data(initial_data);
	}
	array(const ste_context &ctx,
		  std::uint64_t count,
		  const std::vector<T> &initial_data,
		  const VkBufferUsageFlags &usage)
		: ctx(ctx),
		buffer(ctx,
			   count,
			   usage | buffer_usage_additional_flags)
	{
		// Copy initial static data
		copy_initial_data(initial_data);
	}
	~array() noexcept {}

	array(array &&o) noexcept : ctx(o.ctx), buffer(std::move(o.buffer)) {}
	array &operator=(array&&) = default;

	auto size() const { return buffer.get().get_elements_count(); }

	auto& get() { return buffer.get(); }
	auto& get() const { return buffer.get(); }

	operator VkBuffer() const { return get(); }

	auto& operator->() { return get(); }
	auto& operator->() const { return get(); }
	auto& operator*() { return get(); }
	auto& operator*() const { return get(); }
};

}
}
