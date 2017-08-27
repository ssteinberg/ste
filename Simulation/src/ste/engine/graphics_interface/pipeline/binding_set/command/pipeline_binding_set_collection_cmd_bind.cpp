
#include <stdafx.hpp>
#include <pipeline_binding_set_collection_cmd_bind.hpp>

#include <pipeline_binding_set_collection.hpp>

using namespace ste;
using namespace ste::gl;

void pipeline_binding_set_collection_cmd_bind::operator()(const command_buffer &buffer, command_recorder &recorder) const {
	auto &sets = collection->get_sets();
	if (!sets.size())
		return;

	// Bind range of consecutive sets
	lib::vector<const vk::vk_descriptor_set<>*> bind_sets;
	bind_sets.reserve(sets.size());

	for (auto &s : sets) {
		bind_sets.push_back(&s.get());
	}

	// And bind
	recorder << cmd_bind_descriptor_sets(bind_point,
										 *layout,
										 0,
										 bind_sets);
}
