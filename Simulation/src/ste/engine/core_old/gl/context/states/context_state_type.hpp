// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <context_state_name.hpp>

namespace StE {
namespace Core {
namespace GL {

enum class context_state_type : std::int32_t {
	BasicState,
	BufferChange,
	TextureChange,
	FBOChange,
	ProgramChange,
	VAOChange,
	TransformFeedbackActivation,
};

inline context_state_type context_state_type_from_name(const BasicStateName &n) {
	return context_state_type::BasicState;
}

inline context_state_type context_state_type_from_name(const StateName &n) {
	switch (n) {
	case StateName::VIEWPORT_STATE:
	case StateName::COLOR_MASK_STATE:
	case StateName::DEPTH_MASK_STATE:
	case StateName::DEPTH_FUNC_STATE:
	case StateName::CULL_FACE_STATE:
	case StateName::FRONT_FACE_STATE:
	case StateName::BLEND_FUNC_STATE:
	case StateName::BLEND_FUNC_SEPARATE_STATE:
	case StateName::BLEND_COLOR_STATE:
	case StateName::BLEND_EQUATION_STATE:
	case StateName::CLEAR_COLOR_STATE:
	case StateName::CLEAR_DEPTH_STATE:
		return context_state_type::BasicState;

	// Resource states
	case StateName::BUFFER_OBJECT:
		return context_state_type::BufferChange;
	case StateName::FRAMEBUFFER_OBJECT:
	case StateName::RENDERBUFFER_OBJECT:
		return context_state_type::FBOChange;
	case StateName::TEXTURE_OBJECT:
	case StateName::IMAGE_OBJECT:
	case StateName::SAMPLER_OBJECT:
		return context_state_type::TextureChange;
	case StateName::VERTEX_ARRAY_OBJECT:
		return context_state_type::VAOChange;
	case StateName::GLSL_PROGRAM_OBJECT:
		return context_state_type::ProgramChange;
	case StateName::TRANSFORM_FEEDBACK_OBJECT:
		return context_state_type::TransformFeedbackActivation;
	}

	assert(false);
	return context_state_type::BasicState;
}

}
}
}
