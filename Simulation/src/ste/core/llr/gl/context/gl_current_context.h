// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class gl_generic_context;

class gl_current_context {
private:
	friend class gl_generic_context;

private:
	static thread_local gl_generic_context *current;

public:
	static gl_generic_context *get() { return current; }
};

}
}

#include "gl_generic_context.h"
