// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _DEBUG
#define _assert(x) { if (!(x)) {std::cout << #x << std::endl << "Failed" << std::endl; throw new std::exception(#x);} }
#else
#define	_assert(x) assert(x)
#endif

#include <exception>

#define GLM_FORCE_AVX
#define GLM_EXT_INCLUDED
#include <glm/glm.hpp>
