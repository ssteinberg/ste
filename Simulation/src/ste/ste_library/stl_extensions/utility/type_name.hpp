//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <string_view>

namespace ste {

template <class T>
constexpr std::string_view type_name() {
#ifdef __clang__
	string_view p = __PRETTY_FUNCTION__;
	return std::string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
	std::string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
	return std::string_view(p.data() + 36, p.size() - 36 - 1);
#  else
	return std::string_view(p.data() + 46, p.size() - 46 - 1);
#  endif
#elif defined(_MSC_VER)
	std::string_view p = __FUNCSIG__;
	return std::string_view(p.data() + 38, p.size() - 38 - 7);
#endif
}

}
