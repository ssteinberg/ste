//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <wait_semaphore.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class command_buffer;

class command {
	friend class command_recorder;

private:
	lib::vector<wait_semaphore> dependencies;

private:
	virtual void operator()(const command_buffer &command_buffer, command_recorder &) && = 0;

public:
	command() = default;
	virtual ~command() noexcept {}

	command(command&&) = default;
	command(const command&) = default;
	command &operator=(command&&) = default;
	command &operator=(const command&) = default;

	/*
	 *	@brief	Adds a dependency to the command in the form of a wait semaphore. When recorded to a command buffer and submitted as part of a batch,
	 *			the dependency will be added to the wait semaphores.
	 */
	void add_dependency(wait_semaphore &&sem) { dependencies.emplace_back(std::move(sem)); }

	auto extract_dependencies() && { return std::move(dependencies); }
};

}
}
