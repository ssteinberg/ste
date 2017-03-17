//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_resource_pool.hpp>
#include <vk_command_pool.hpp>

namespace StE {
namespace GL {

class ste_device_queue_command_pool : public ste_resource_pool_resetable_trait<const vk_logical_device &, std::uint32_t, VkCommandPoolCreateFlags>{
private:
	static constexpr int releases_resource_every = 100;

	vk_command_pool pool;
	std::uint32_t counter{ 0 };

public:
	template <typename... Args>
	ste_device_queue_command_pool(Args&&... args) : pool(std::forward<Args>(args)...) {}
	ste_device_queue_command_pool(ste_device_queue_command_pool&&) = default;
	ste_device_queue_command_pool &operator=(ste_device_queue_command_pool&&) = default;

	auto& get_pool() { return pool; }
	auto& get_pool() const { return pool; }

	void reset() override {
		(++counter % releases_resource_every == 0) ?
			pool.reset_release() :
			pool.reset();
	}
};

}
}
