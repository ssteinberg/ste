//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_shader_stage_attachment.hpp>

namespace ste {
namespace gl {

/**
*	@brief	Describes layout of a single output attachment in a pipeline
*/
struct pipeline_attachment_layout {
	const ste_shader_stage_attachment* attachment;

	/**
	*	@brief	Helper method to get underlying variable name
	*/
	const lib::string& name() const { return attachment->variable->name(); }

	/**
	*	@brief	Helper method to get the attachment's location
	*/
	auto location() const { return attachment->location; }
};

}
}
