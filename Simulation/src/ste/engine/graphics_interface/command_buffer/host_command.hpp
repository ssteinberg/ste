//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <vk_queue.hpp>

#include <functional>

namespace StE {
namespace GL {

class host_command {
private:
	using command_t = std::function<void(const vk_queue &)>;

private:
	command_t cmd;

public:
	host_command(command_t &&cmd) : cmd(std::move(cmd)) {}

	host_command(host_command&&) = default;
	host_command &operator=(host_command&&) = default;

	auto operator()(const vk_queue &queue) const {
		return cmd(queue);
	}
};

}
}
