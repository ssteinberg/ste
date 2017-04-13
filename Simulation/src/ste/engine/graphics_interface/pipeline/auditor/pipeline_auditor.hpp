//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <device_pipeline.hpp>
#include <pipeline_layout.hpp>

#include <vector>

namespace StE {
namespace GL {

class pipeline_auditor {
public:
	using shader_stage_t = pipeline_layout::shader_stage_t;

private:
	virtual std::vector<shader_stage_t> stages() const = 0;

public:
	pipeline_auditor() = default;
	virtual ~pipeline_auditor() noexcept {}

	virtual std::unique_ptr<device_pipeline> pipeline(const ste_context &ctx) const = 0;
};

}
}
