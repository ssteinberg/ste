//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <vk_logical_device.hpp>

#include <pipeline_binding_layout_interface.hpp>
#include <binding_set_pool_instance.hpp>

#include <atomic>
#include <lib/concurrent_unordered_map.hpp>
#include <lib/intrusive_ptr.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

class pipeline_binding_set_pool : anchored {
private:
	using pool_key = std::uint32_t;
	using instance_t = binding_set_pool_instance<pipeline_binding_set_pool, pool_key>;
	using pool_ptr_t = lib::intrusive_ptr<instance_t>;
	using pools_t = lib::concurrent_unordered_map<pool_key, pool_ptr_t>;

	friend instance_t;

private:
	alias<const vk::vk_logical_device> device;

	pools_t pools;
	std::atomic<pool_key> key_counter{ 0 };

private:
	template <typename Layout>
	auto allocate_pool_instance(const lib::vector<const Layout*> &layouts,
								const pool_key &key) {
		auto sets_count = layouts.size();
		lib::vector<const pipeline_binding_layout_interface*> pool_bindings;
		pool_bindings.reserve(sets_count * 10);

		// Copy bindings
		for (auto &l : layouts) {
			auto &layout = *l;
			for (std::size_t i=0; i<layout.size(); ++i)
				pool_bindings.push_back(&layout[i]);
		}

		// Max sets
		auto max_sets = sets_count;

		// Allocate
		return pool_ptr_t(lib::allocate_intrusive<instance_t>(instance_t::ctor(),
															  device.get(),
															  static_cast<std::uint32_t>(max_sets),
															  pool_bindings,
															  this,
															  key));
	}

	void release_one(const pool_key &k) {
		pools.remove(k);
	}

public:
	pipeline_binding_set_pool(const vk::vk_logical_device &device)
		: device(device)
	{}
	~pipeline_binding_set_pool() noexcept {}

	/**
	*	@brief	Allocates the required layouts.
	*			Allocation is thread-safe. The returned sets are not thread-safe are assumed to exist on and accessed
	*			from a single thread only.
	*/
	template <typename Layout>
	auto allocate_binding_sets(const lib::vector<const Layout*> &layouts) {
		// Create a key
		auto key = key_counter.fetch_add(1);

		// Create new slot
		auto instance = allocate_pool_instance<Layout>(layouts,
													   key);

		// Allocate
		auto sets = instance->allocate<Layout>(layouts);

		// Insert pool instance into pools
		pools.emplace(key, std::move(instance));

		return sets;
	}
};

}
}
