//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <fence.hpp>
#include <ste_device_queue_command_pool.hpp>

#include <ste_resource_pool.hpp>
#include <ste_device_queue_command_buffer.hpp>

#include <memory>

namespace StE {
namespace GL {

class ste_device_queue_batch {
	friend class ste_device_queue;

private:
	using pool_t = ste_resource_pool<ste_device_queue_command_pool>::resource_t;
	using fence_t = ste_resource_pool<fence<void>>::resource_t;

private:
	pool_t pool;
	std::shared_ptr<fence_t> fence_strong;

	std::uint32_t queue_index;

	std::vector<ste_device_queue_command_buffer> command_buffers;

	auto begin() const { return std::begin(command_buffers); }
	auto end() const { return std::end(command_buffers); }

public:
	ste_device_queue_batch(std::uint32_t queue_index, 
						   pool_t &&pool, 
						   fence_t &&f)
		: pool(std::move(pool)), 
		fence_strong(std::make_shared<fence_t>(std::move(f))),
		queue_index(queue_index)
	{}

	ste_device_queue_batch(ste_device_queue_batch&&) = default;
	ste_device_queue_batch &operator=(ste_device_queue_batch&&) = default;

	auto& acquire_command_buffer() {
		command_buffers.emplace_back(pool->get_pool(), vk_command_buffer_type::primary);
		return command_buffers.back();
	}

	auto& get_fence() const {
		return fence_strong;
	}
};

}
}
