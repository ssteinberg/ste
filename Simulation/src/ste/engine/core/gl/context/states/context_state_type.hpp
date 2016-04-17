// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "context_state_name.hpp"

namespace StE {
namespace Core {

enum class context_state_type : std::int32_t {
	BasicState,
	BufferChange,
	TextureChange,
	FBOChange,
	ProgramChange,
	VAOChange,
	TransformFeedbackActivation,
};

inline context_state_type context_state_type_from_name(const context_state_name &n) {
	switch (n) {
	case context_state_name::BLEND:
	case context_state_name::CLIP_PLANE0:
	case context_state_name::CLIP_PLANE1:
	case context_state_name::CLIP_PLANE2:
	case context_state_name::CLIP_PLANE3:
	case context_state_name::CLIP_PLANE4:
	case context_state_name::CLIP_PLANE5:
	case context_state_name::COLOR_LOGIC_OP:
	case context_state_name::CULL_FACE:
	case context_state_name::DEPTH_TEST:
	case context_state_name::DITHER:
	case context_state_name::FRAMEBUFFER_SRGB:
	case context_state_name::LINE_SMOOTH:
	case context_state_name::MULTISAMPLE:
	case context_state_name::POLYGON_OFFSET_FILL:
	case context_state_name::POLYGON_OFFSET_LINE:
	case context_state_name::POLYGON_OFFSET_POINT:
	case context_state_name::POLYGON_SMOOTH:
	case context_state_name::SAMPLE_ALPHA_TO_COVERAGE:
	case context_state_name::SAMPLE_ALPHA_TO_ONE:
	case context_state_name::SAMPLE_COVERAGE:
	case context_state_name::SCISSOR_TEST:
	case context_state_name::STENCIL_TEST:
	case context_state_name::TEXTURE_CUBE_MAP_SEAMLESS:
	case context_state_name::VERTEX_PROGRAM_POINT_SIZE:
		return context_state_type::BasicState;

	// States
	case context_state_name::VIEWPORT_STATE:
	case context_state_name::COLOR_MASK_STATE:
	case context_state_name::DEPTH_MASK_STATE:
	case context_state_name::CULL_FACE_STATE:
	case context_state_name::FRONT_FACE_STATE:
	case context_state_name::BLEND_FUNC_STATE:
	case context_state_name::BLEND_FUNC_SEPARATE_STATE:
	case context_state_name::BLEND_COLOR_STATE:
	case context_state_name::BLEND_EQUATION_STATE:
	case context_state_name::CLEAR_COLOR_STATE:
	case context_state_name::CLEAR_DEPTH_STATE:
		return context_state_type::BasicState;

	// Resource states
	case context_state_name::BUFFER_OBJECT:
		return context_state_type::BufferChange;
	case context_state_name::FRAMEBUFFER_OBJECT:
	case context_state_name::RENDERBUFFER_OBJECT:
		return context_state_type::FBOChange;
	case context_state_name::TEXTURE_OBJECT:
	case context_state_name::IMAGE_OBJECT:
	case context_state_name::SAMPLER_OBJECT:
		return context_state_type::TextureChange;
	case context_state_name::VERTEX_ARRAY_OBJECT:
		return context_state_type::VAOChange;
	case context_state_name::GLSL_PROGRAM_OBJECT:
		return context_state_type::ProgramChange;
	case context_state_name::TRANSFORM_FEEDBACK_OBJECT:
		return context_state_type::TransformFeedbackActivation;
	}

	assert(false);
	return context_state_type::BasicState;
}

}
}
