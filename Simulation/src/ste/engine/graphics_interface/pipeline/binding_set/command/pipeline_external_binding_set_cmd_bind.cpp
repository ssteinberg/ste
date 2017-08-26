
#include <stdafx.hpp>
#include <pipeline_external_binding_set_cmd_bind.hpp>

#include <pipeline_external_binding_set.hpp>

using namespace ste;
using namespace ste::gl;

void pipeline_external_binding_set_cmd_bind::operator()(const command_buffer &buffer, command_recorder &recorder) const {
	// Bind set
	const lib::vector<const vk::vk_descriptor_set<>*> bind_sets = { &set->get_set().get() };
	recorder << cmd_bind_descriptor_sets(bind_point,
										 *layout,
										 0,
										 bind_sets);
}
