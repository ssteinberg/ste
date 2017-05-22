//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <device_pipeline_exceptions.hpp>
#include <pipeline_binding_layout_interface.hpp>

#include <vk_descriptor_set.hpp>

#include <allow_type_decay.hpp>
#include <ultimate.hpp>
#include <range_list.hpp>
#include <alias.hpp>

namespace ste {
namespace gl {

namespace _internal {

/**
 *	@brief	Defines a binding set.
 *			Layout must be a name->const pipeline_binding_base* map
 */
template <typename Layout>
class pipeline_binding_set_impl : public allow_type_decay<pipeline_binding_set_impl<Layout>, vk::vk_descriptor_set> {
private:
	using binding_to_written_descriptors_range_map = lib::flat_map<std::uint32_t, range_list<std::uint32_t>>;

private:
	vk::vk_descriptor_set set;
	alias<const Layout> layout;

	ultimate_type_erasure last_act;

	// Keeps track of the written resource descriptors to the binding set
	binding_to_written_descriptors_range_map written_descriptors;

private:
	/**
	 *	@brief	Creates the resource copy information to copy bindings from another binding sets.
	 */
	lib::vector<vk::vk_descriptor_set_copy_resources> create_descriptor_set_copy_information(const pipeline_binding_set_impl &o) {
		const auto& dst_bindings = layout.get();
		const auto& src_bindings = o.layout.get();

		auto binds_to_copy = std::min(src_bindings.size(), dst_bindings.size());

		// Allocate memory for copy descriptors
		lib::vector<vk::vk_descriptor_set_copy_resources> copies;
		copies.reserve(binds_to_copy * 5);

		// Create copy descriptors
		for (const pipeline_binding_layout_interface *dst : dst_bindings) {
			// Read binding properties
			auto &name = dst->name();
			auto bind_idx = dst->bind_idx();

			// Read range list of copyable descriptors at the source's binding index
			auto it = o.written_descriptors.find(bind_idx);
			if (it == o.written_descriptors.end())
				continue;
			auto &src_ranges = it->second;

			// Find source (find a binding with same name at source layout)
			if (!src_bindings.exists(name)) {
				throw device_pipeline_incompatible_binding_sets_exception("No source binding found");
			}
			const pipeline_binding_layout_interface* src = &src_bindings[name];

			// Verify
			if (dst->vk_descriptor_type() != src->vk_descriptor_type() ||
				dst->stage_collection() != src->stage_collection()) {
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

				copies.push_back(vk::vk_descriptor_set_copy_resources(o.set,
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
	pipeline_binding_set_impl(vk::vk_descriptor_set &&set,
							  const Layout &layout,
							  ultimate<F>&& last_act) :
		set(std::move(set)),
		layout(layout),
		last_act(std::move(last_act))
	{}
	~pipeline_binding_set_impl() noexcept {}

	/**
	*	@brief	See copy() for more information
	*/
	template <typename F>
	pipeline_binding_set_impl(vk::vk_descriptor_set &&set,
							  const Layout &layout,
							  ultimate<F>&& last_act,
							  const pipeline_binding_set_impl &o)
		: pipeline_binding_set_impl(std::move(set),
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
	void copy(const pipeline_binding_set_impl &o) {
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
	void write(const lib::vector<vk::vk_descriptor_set_write_resource> &writes) {
		set.write(writes);

		for (auto &w : writes) {
			auto bind_idx = w.get_binding_index();
			auto start = w.get_array_element();
			auto count = w.get_count();

			written_descriptors[bind_idx].add({ start, count });
		}
	}

	pipeline_binding_set_impl(pipeline_binding_set_impl&&) = default;
	pipeline_binding_set_impl &operator=(pipeline_binding_set_impl&&) = default;

	auto& get() const { return set; }

	/**
	 *	@brief	Returns the set's layout
	 */
	auto &get_layout() const { return layout.get(); }

	/**
	*	@brief	Returns the binding set index
	*/
	auto &get_set_index() const {
		return layout.get().get_set_index();
	}
};

}

}
}
