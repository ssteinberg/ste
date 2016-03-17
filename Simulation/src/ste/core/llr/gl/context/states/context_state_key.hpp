// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "context_state_name.hpp"
#include "hash_combine.hpp"

namespace StE {
namespace LLR {
	
class context_state_key {
private:
	friend class std::hash<context_state_key>;

private:
	context_state_name name;
	std::uint32_t index0{ 0 }, index1{ 0 };

public:
	context_state_key(context_state_name name): name(name) {}
	context_state_key(context_state_name name, std::uint32_t index): name(name), index0(index) {}
	context_state_key(context_state_name name, std::uint32_t index0, std::uint32_t index1): name(name), index0(index0), index1(index1) {}
	
	bool operator==(const context_state_key &k) const {
		return name == k.name && index0 == k.index0 && index1 == k.index1;
	}
	bool operator!=(const context_state_key &k) const {
		return !((*this) == k);
	}
};
	
}
}


namespace std {

template <> struct hash<StE::LLR::context_state_key> {
	size_t inline operator()(const StE::LLR::context_state_key &x) const {
		std::int64_t h1 = static_cast<std::int64_t>(x.index0) + (static_cast<std::int64_t>(x.index1) << 32);
		
		using T = typename std::underlying_type<decltype(x.name)>::type;
		
		auto n = static_cast<T>(x.name);
		return StE::hash_combine(std::hash<std::int64_t>()(h1), std::hash<T>()(n));
	}
};

}

