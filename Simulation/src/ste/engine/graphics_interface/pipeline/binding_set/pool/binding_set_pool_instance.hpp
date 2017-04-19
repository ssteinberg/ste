//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <vk_descriptor_pool.hpp>

#include <pipeline_binding_layout_interface.hpp>
#include <pipeline_binding_set_impl.hpp>
#include <pipeline_binding_layout.hpp>

#include <vector>
#include <ultimate.hpp>

namespace StE {
namespace GL {

class binding_set_pool_instance {
	friend class pipeline_binding_set_pool;

	struct ctor {};

public:
	template <typename Layout>
	using allocation_result_t = std::vector<_internal::pipeline_binding_set_impl<Layout>>;
	using release_func_t = std::function<void(void)>;

private:
	const ste_context &ctx;
	vk_descriptor_pool pool;

	std::uint32_t allocated_sets{ 0 };

	release_func_t release_func;

private:
	/**
	 *	@brief	Creates the Vulkan binding descriptors
	 */
	auto create_vk_bindings(const std::vector<const pipeline_binding_layout_interface*> &pool_bindings) {
		std::vector<vk_descriptor_set_layout_binding> vk_bindings;
		vk_bindings.reserve(pool_bindings.size());
		for (auto &b : pool_bindings) {
			vk_bindings.push_back(*b);
		}

		return vk_bindings;
	}

	void release_one() {
		--allocated_sets;
		if (allocated_sets == 0) {
			// Release pool
			release_func();
		}
	}

public:
	binding_set_pool_instance(ctor,
							  const ste_context &ctx,
							  std::uint32_t max_sets,
							  const std::vector<const pipeline_binding_layout_interface*> &pool_bindings)
		: ctx(ctx),
		pool(ctx.device(),
			 max_sets,
			 create_vk_bindings(pool_bindings),
			 false)
	{}
	~binding_set_pool_instance() noexcept {}

	binding_set_pool_instance(binding_set_pool_instance&&) = default;
	binding_set_pool_instance &operator=(binding_set_pool_instance&&) = default;

	/**
	 *	@brief	Attempts to allocate the required layouts
	 *	
	 *	@throws	vk_exception	On Vulkan exception
	 */
	template <typename Layout>
	auto allocate(const std::vector<const Layout*> &layouts) {
		std::vector<const vk_descriptor_set_layout*> layouts_of_acquired_bindings_ptrs;
		layouts_of_acquired_bindings_ptrs.reserve(layouts.size());
		for (std::size_t i = 0; i < layouts.size(); ++i) {
			auto &l = *layouts[i];
			layouts_of_acquired_bindings_ptrs.push_back(&l.get());
		}

		// Allocate
		std::vector<vk_descriptor_set> sets = pool.allocate_descriptor_sets(layouts_of_acquired_bindings_ptrs);

		// Sets allocated successfully
		allocated_sets += static_cast<std::uint32_t>(layouts.size());

		allocation_result_t<Layout> ret;
		ret.reserve(sets.size());
		for (std::size_t i = 0; i < sets.size(); ++i) {
			auto layout = layouts[i];
			auto last_act = ultimately([this, layout]() {
				this->release_one();
			});

			ret.push_back(_internal::pipeline_binding_set_impl<Layout>(std::move(sets[i]),
																	   *layout,
																	   std::move(last_act)));
		}

		return ret;
	}
};

}
}
