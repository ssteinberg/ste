//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace gl {

template <class Command>
struct task_policy {};

struct task_policy_common {
	template <typename Command, typename Task, typename... CmdArgs>
	static auto create_cmd(const Task *task,
					CmdArgs&&... args) {
		return Command(std::forward<CmdArgs>(args)...);
	}
};

}
}

#include <task_policy_draw.hpp>
#include <task_policy_compute.hpp>
#include <task_policy_transfer.hpp>
