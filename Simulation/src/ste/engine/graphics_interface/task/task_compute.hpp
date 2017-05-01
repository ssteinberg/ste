//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <task.hpp>
#include <job.hpp>

#include <device_pipeline_compute.hpp>

namespace ste {
namespace gl {

class task_compute : public command {
public:
	task_compute()
	virtual ~task_compute() noexcept {}

	task_compute(task_compute&&) = default;
	task_compute &operator=(task_compute&&) = default;
};

}
}
