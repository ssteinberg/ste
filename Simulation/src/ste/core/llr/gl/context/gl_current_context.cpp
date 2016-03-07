
#include "stdafx.h"
#include "gl_current_context.h"

thread_local StE::LLR::gl_context *StE::LLR::gl_current_context::current = nullptr;
