
#include "stdafx.hpp"
#include "gl_current_context.hpp"

thread_local StE::LLR::gl_generic_context *StE::LLR::gl_current_context::current = nullptr;
