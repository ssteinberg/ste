//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <command_buffer.hpp>
#include <command_recorder.hpp>
#include <cmd_bind_descriptor_sets.hpp>

namespace ste {
namespace gl {

// Bind command
template <class SetCollection>
class pipeline_binding_set_collection_cmd_bind : public command {
	const SetCollection *collection;
	VkPipelineBindPoint bind_point;
	const vk::vk_pipeline_layout *layout;

public:
	pipeline_binding_set_collection_cmd_bind(const SetCollection *collection,
											 VkPipelineBindPoint bind_point,
											 const vk::vk_pipeline_layout *layout)
		: collection(collection),
		bind_point(bind_point),
		layout(layout)
	{}
	virtual ~pipeline_binding_set_collection_cmd_bind() noexcept {}

private:
	void operator()(const command_buffer &buffer, command_recorder &recorder) const override final {
		auto &sets = collection->get_sets();
		if (!sets.size())
			return;

		std::uint32_t first_set_idx = sets.begin()->first;

		// Bind ranges of consecutive sets
		for (auto it = sets.begin(); it != sets.end();) {
			std::vector<const vk::vk_descriptor_set*> bind_sets;
			bind_sets.reserve(sets.size());

			// Create a range
			auto set_idx = it->first;
			bind_sets.push_back(&it->second.get());
			++it;
			for (; it != sets.end() && it->first == set_idx + 1; ++it, ++set_idx)
				bind_sets.push_back(&it->second.get());

			// And bind
			recorder << cmd_bind_descriptor_sets(bind_point,
												 *layout,
												 first_set_idx,
												 bind_sets);
		}
	}
};

}
}
