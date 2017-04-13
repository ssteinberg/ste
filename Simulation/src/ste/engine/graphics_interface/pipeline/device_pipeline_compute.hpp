//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>

namespace StE {
namespace GL {

class device_pipeline_compute : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_compute;

private:
	struct ctor {};

public:
	device_pipeline_compute(ctor,
							const ste_context &ctx,
							pipeline_layout &&layout)
		: Base(ctx, std::move(layout))
	{}
	~device_pipeline_compute() noexcept {}
};

}
}
