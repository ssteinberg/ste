
#include <stdafx.hpp>
#include <command_recorder.hpp>
#include <command_buffer.hpp>
#include <command.hpp>

using namespace ste::gl;

void command_recorder::end() {
	if (buffer) {
		buffer->end();
		buffer = nullptr;
	}
}

command_recorder &command_recorder::operator<<(command &&cmd) {
	// Record command
	std::move(cmd)(*buffer, *this);

	// Append command dependencies to command buffer
	auto deps = std::move(cmd).extract_dependencies();
	buffer->dependencies.reserve(buffer->dependencies.size() + deps.size());
	buffer->dependencies.insert(buffer->dependencies.end(), 
								std::make_move_iterator(deps.begin()), 
								std::make_move_iterator(deps.end()));

	return *this;
}
