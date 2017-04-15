//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_layout.hpp>

#include <device_pipeline_exceptions.hpp>

#include <vk_descriptor_set.hpp>
#include <allow_type_decay.hpp>
#include <ultimate.hpp>

namespace StE {
namespace GL {

class pipeline_binding_set : public allow_type_decay<pipeline_binding_set, vk_descriptor_set> {
private:
	vk_descriptor_set set;
	std::reference_wrapper<const pipeline_binding_set_layout> layout;

	ultimate_type_erasure last_act;

private:
	/**
	 *	@brief	Creates the resource copy information to copy bindings from another binding sets.
	 */
	auto create_descriptor_set_copy_information(const pipeline_binding_set &o) {
		const auto& dst_vk_bindings = layout.get().get_vk_bindings();
		const auto& src_vk_bindings = o.layout.get().get_vk_bindings();

		auto binds_to_copy = std::min(src_vk_bindings.size(), dst_vk_bindings.size());

		std::vector<vk_descriptor_set_copy_resources> copies;
		copies.reserve(binds_to_copy);
		for (decltype(binds_to_copy) i = 0; i<binds_to_copy; ++i) {
			if (dst_vk_bindings[i].get_type() != src_vk_bindings[i].get_type() ||
				dst_vk_bindings[i].get_stage() != src_vk_bindings[i].get_stage()) {
				throw device_pipeline_incompatible_binding_sets_exception("Source and destination binding type mismatch");
			}

			auto count = std::min(src_vk_bindings[i].get_count(),
								  dst_vk_bindings[i].get_count());
			vk_descriptor_set_copy_resources copy(o.set,
												  src_vk_bindings[i].get_index(),
												  0,
												  dst_vk_bindings[i].get_index(),
												  0,
												  count);
			copies.push_back(copy);
		}

		return copies;
	}

public:
	template <typename F>
	pipeline_binding_set(vk_descriptor_set &&set,
						 const pipeline_binding_set_layout &layout,
						 ultimate<F>&& last_act) :
		set(std::move(set)),
		layout(layout),
		last_act(std::move(last_act))
	{}
	~pipeline_binding_set() noexcept {}

	/**
	*	@brief	See copy() for more information
	*/
	template <typename F>
	pipeline_binding_set(vk_descriptor_set &&set,
						 const pipeline_binding_set_layout &layout,
						 ultimate<F>&& last_act,
						 const pipeline_binding_set &o)
		: pipeline_binding_set(std::move(set),
							   layout,
							   std::move(last_act))
	{
		copy(o);
	}

	/**
	*	@brief	Copies bindings from another set and writes them to the descriptor.
	*			Assumes the layout are similar, with only binding count (array length) changes.
	*
	*	@throws	device_pipeline_incompatible_binding_sets_exception		If the above assumption is violated
	*/
	void copy(const pipeline_binding_set &o) {
		auto copies = create_descriptor_set_copy_information(o);
		set.copy(copies);
	}

	/**
	 *	@brief	Updates binding set resource bindings
	 */
	void write(const std::vector<vk_descriptor_set_write_resource> &writes) {
		set.write(writes);
	}

	pipeline_binding_set(pipeline_binding_set&&) = default;
	pipeline_binding_set &operator=(pipeline_binding_set&&) = default;

	auto& get() const { return set; }
};

}
}
