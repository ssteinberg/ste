//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <pipeline_layout.hpp>

#include <lib/vector.hpp>

namespace ste {
namespace gl {

class pipeline_auditor {
public:
	using shader_stage_t = pipeline_layout::shader_stage_t;

private:
	virtual lib::vector<shader_stage_t> stages() const = 0;

public:
	pipeline_auditor() = default;
	virtual ~pipeline_auditor() noexcept {}
};

}
}
