// StE
// � Shlomi Steinberg, 2015

#pragma once

#ifdef DEBUG
#define	ENABLE_TRACE
#endif

#ifdef ENABLE_TRACE
#  ifdef _MSC_VER
#	 include "windows.h"
#    include <sstream>
#    define TRACE(x) {  std::stringstream s;  s << (x); OutputDebugString(s.str().c_str()); }
#  else
#    include <iostream>
#    define TRACE(x)  { std::cerr << (x) << std::flush; }
#  endif
#else
#  define TRACE(x)
#endif
