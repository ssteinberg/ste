//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_binding_set_layout.hpp>

#include <device_pipeline_exceptions.hpp>

#include <vk_descriptor_set.hpp>
#include <allow_type_decay.hpp>
#include <ultimate.hpp>

#include <range.hpp>
#include <range_list.hpp>

namespace StE {
namespace GL {

class pipeline_binding_set : public allow_type_decay<pipeline_binding_set, vk_descriptor_set> {
private:
	using binding_to_written_descriptors_range_map = boost::container::flat_map<std::uint32_t, range_list<std::uint32_t>>;

private:
	vk_descriptor_set set;
	std::reference_wrapper<const pipeline_binding_set_layout> layout;

	ultimate_type_erasure last_act;

	// Keeps track of the written resource descriptors to the binding set
	binding_to_written_descriptors_range_map written_descriptors;

private:
	/**
	 *	@brief	Creates the resource copy information to copy bindings from another binding sets.
	 */
	std::vector<vk_descriptor_set_copy_resources> create_descriptor_set_copy_information(const pipeline_binding_set &o) {
		const auto& dst_bindings = layout.get();
		const auto& src_bindings = o.layout.get();

		auto binds_to_copy = std::min(src_bindings.size(), dst_bindings.size());

		// Allocate memory for copy descriptors
		std::vector<vk_descriptor_set_copy_resources> copies;
		copies.reserve(binds_to_copy * 5);

		// Create copy descriptors
		for (auto &dst : dst_bindings) {
			// Read binding properties
			auto &name = dst->name();
			auto &bind_idx = dst->bind_idx();

			// Read range list of copyable descriptors at the source's binding index
			auto it = o.written_descriptors.find(bind_idx);
			if (it == o.written_descriptors.end())
				continue;
			auto &src_ranges = it->second;

			// Find source (find a binding with same name at source layout)
			const pipeline_binding_set_layout_binding* src = src_bindings[name];
			if (src == nullptr) {
				throw device_pipeline_incompatible_binding_sets_exception("No source binding found");
			}

			// Verify
			if (dst->vk_descriptor_type() != src->vk_descriptor_type() ||
				static_cast<VkShaderStageFlags>(dst->stages) != static_cast<VkShaderStageFlags>(src->stages)) {
				assert(false && "Verification failed");
				return {};
			}

			// Generate copy descriptors
			for (auto &r : src_ranges) {
				auto max_count = std::min(src->count(),
										  dst->count());

				auto start = r.start;
				if (start >= max_count)
					continue;

				auto count = std::min<std::uint32_t>(max_count - start, r.length);

				copies.push_back(vk_descriptor_set_copy_resources(o.set,
																  src->bind_idx(),
																  start,
																  bind_idx,
																  start,
																  count));
			}
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

		for (auto &c : copies) {
			auto bind_idx = c.get_dst_binding_index();
			auto start = c.get_dst_array_element();
			auto count = c.get_count();

			written_descriptors[bind_idx].add({ start, count });
		}
	}

	/**
	 *	@brief	Updates binding set resource bindings
	 */
	void write(const std::vector<vk_descriptor_set_write_resource> &writes) {
		set.write(writes);

		for (auto &w : writes) {
			auto bind_idx = w.get_binding_index();
			auto start = w.get_array_element();
			auto count = w.get_count();

			written_descriptors[bind_idx].add({ start, count });
		}
	}

	pipeline_binding_set(pipeline_binding_set&&) = default;
	pipeline_binding_set &operator=(pipeline_binding_set&&) = default;

	auto& get() const { return set; }
};

}
}
