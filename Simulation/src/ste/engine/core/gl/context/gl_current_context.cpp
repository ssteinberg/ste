
#include "stdafx.hpp"
#include "gl_current_context.hpp"

thread_local StE::Core::gl_generic_context *StE::Core::gl_current_context::current = nullptr;
