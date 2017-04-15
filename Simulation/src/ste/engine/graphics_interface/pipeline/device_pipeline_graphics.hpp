//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_pipeline.hpp>

namespace StE {
namespace GL {

class device_pipeline_graphics : public device_pipeline {
	using Base = device_pipeline;

	friend class pipeline_auditor_graphics;

private:
	struct ctor {};

public:
	device_pipeline_graphics(ctor,
							 const ste_context &ctx,
							 pipeline_binding_set_pool &pool,
							 pipeline_layout &&layout)
		: Base(ctx,
			   pool,
			   std::move(layout))
	{}
	~device_pipeline_graphics() noexcept {}
};

}
}
