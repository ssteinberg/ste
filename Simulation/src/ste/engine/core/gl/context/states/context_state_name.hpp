// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include <type_traits>

namespace StE {
namespace Core {
namespace GL {

enum class BasicStateName : std::int32_t {
	// Basic states (boolean glenable/gldisable states)
	BLEND = GL_BLEND,
	CLIP_PLANE0 = GL_CLIP_PLANE0,
	CLIP_PLANE1 = GL_CLIP_PLANE1,
	CLIP_PLANE2 = GL_CLIP_PLANE2,
	CLIP_PLANE3 = GL_CLIP_PLANE3,
	CLIP_PLANE4 = GL_CLIP_PLANE4,
	CLIP_PLANE5 = GL_CLIP_PLANE5,
	COLOR_LOGIC_OP = GL_COLOR_LOGIC_OP,
	CULL_FACE = GL_CULL_FACE,
	DEPTH_TEST = GL_DEPTH_TEST,
	DITHER = GL_DITHER,
	FRAMEBUFFER_SRGB = GL_FRAMEBUFFER_SRGB,
	LINE_SMOOTH = GL_LINE_SMOOTH,
	MULTISAMPLE = GL_MULTISAMPLE,
	POLYGON_OFFSET_FILL = GL_POLYGON_OFFSET_FILL,
	POLYGON_OFFSET_LINE = GL_POLYGON_OFFSET_LINE,
	POLYGON_OFFSET_POINT = GL_POLYGON_OFFSET_POINT,
	POLYGON_SMOOTH = GL_POLYGON_SMOOTH,
	SAMPLE_ALPHA_TO_COVERAGE = GL_SAMPLE_ALPHA_TO_COVERAGE,
	SAMPLE_ALPHA_TO_ONE = GL_SAMPLE_ALPHA_TO_ONE,
	SAMPLE_COVERAGE = GL_SAMPLE_COVERAGE,
	SCISSOR_TEST = GL_SCISSOR_TEST,
	STENCIL_TEST = GL_STENCIL_TEST,
	TEXTURE_CUBE_MAP_SEAMLESS = GL_TEXTURE_CUBE_MAP_SEAMLESS,
	VERTEX_PROGRAM_POINT_SIZE = GL_VERTEX_PROGRAM_POINT_SIZE,
};

enum class StateName : std::int32_t {
	// States
	VIEWPORT_STATE,
	COLOR_MASK_STATE,
	DEPTH_MASK_STATE,
	CULL_FACE_STATE,
	FRONT_FACE_STATE,
	BLEND_FUNC_STATE,
	BLEND_FUNC_SEPARATE_STATE,
	BLEND_COLOR_STATE,
	BLEND_EQUATION_STATE,
	CLEAR_COLOR_STATE,
	CLEAR_DEPTH_STATE,

	SAMPLER_OBJECT,
	TRANSFORM_FEEDBACK_OBJECT,

	// Resource states
	BUFFER_OBJECT,
	TEXTURE_OBJECT,
	FRAMEBUFFER_OBJECT,
	IMAGE_OBJECT,
	RENDERBUFFER_OBJECT,
	VERTEX_ARRAY_OBJECT,
	GLSL_PROGRAM_OBJECT,
};

}
}
}

namespace std {

template <> struct hash<StE::Core::GL::BasicStateName> {
	size_t inline operator()(const StE::Core::GL::BasicStateName &x) const {
		using T = typename std::underlying_type<typename std::decay<decltype(x)>::type>::type;
		return std::hash<T>()(static_cast<T>(x));
	}
};

template <> struct hash<StE::Core::GL::StateName> {
	size_t inline operator()(const StE::Core::GL::StateName &x) const {
		using T = typename std::underlying_type<typename std::decay<decltype(x)>::type>::type;
		return std::hash<T>()(static_cast<T>(x));
	}
};

}
