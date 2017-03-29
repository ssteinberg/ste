//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command_buffer.hpp>
#include <cmd_execute_commands.hpp>

#include <memory>

namespace StE {
namespace GL {

class job {
public:
	job() = default;
	virtual ~job() noexcept {}

	job(job&&) = default;
	job &operator=(job&&) = default;

	virtual const command& cmd() const = 0;
};

class static_job : public job {
private:
	std::unique_ptr<command_buffer> buffer;

public:
	static_job(std::unique_ptr<command_buffer> &&buffer) : buffer(std::move(buffer)) {
		assert(buffer->get_type() == vk_command_buffer_type::secondary);
	}
	virtual ~static_job() noexcept {}

	static_job(static_job&&) = default;
	static_job &operator=(static_job&&) = default;

	const command& cmd() const override final {
		return cmd_execute_commands({ *buffer });
	}
};

}
}
