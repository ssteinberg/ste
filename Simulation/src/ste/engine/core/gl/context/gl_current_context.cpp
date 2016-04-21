
#include "stdafx.hpp"
#include "gl_current_context.hpp"

thread_local StE::Core::GL::gl_generic_context *StE::Core::GL::gl_current_context::current = nullptr;
