//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>
#include <ste_resource_pool.hpp>
#include <command_pool.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class ste_device_queue_command_pool : 
	public ste_resource_pool_resetable_trait<const vk::vk_logical_device &, ste_queue_descriptor>,
	public allow_type_decay<ste_device_queue_command_pool, command_pool>
{
private:
	static constexpr int releases_resource_every = 100;

	command_pool pool;
	std::uint32_t counter{ 0 };

public:
	template <typename... Args>
	ste_device_queue_command_pool(Args&&... args) : pool(std::forward<Args>(args)...) {}
	ste_device_queue_command_pool(ste_device_queue_command_pool&&) = default;
	ste_device_queue_command_pool &operator=(ste_device_queue_command_pool&&) = default;

	auto& get() { return pool; }
	auto& get() const { return pool; }

	void reset() override {
		(++counter % releases_resource_every == 0) ?
			pool.reset_release() :
			pool.reset();
	}
};

}
}
