
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

command_recorder& command_recorder::operator<<(const command &cmd) {
	cmd(*buffer, *this);
	return *this;
}

command_recorder& command_recorder::operator<<(host_command &&cmd) {
	buffer->push_command(std::move(cmd));
	return *this;
}
