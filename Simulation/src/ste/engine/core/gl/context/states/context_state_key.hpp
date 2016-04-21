// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "hash_combine.hpp"

namespace StE {
namespace Core {
namespace GL {

template <typename K>
class context_state_key {
private:
	friend class std::hash<context_state_key>;

private:
	K name;
	std::uint32_t index0{ 0 }, index1{ 0 };

public:
	context_state_key(const K &name): name(name) {}
	context_state_key(const K &name, std::uint32_t index): name(name), index0(index) {}
	context_state_key(const K &name, std::uint32_t index0, std::uint32_t index1): name(name), index0(index0), index1(index1) {}

	context_state_key(const context_state_key &) = default;
	context_state_key(context_state_key &&) = default;
	context_state_key &operator=(const context_state_key &) = default;
	context_state_key &operator=(context_state_key &&) = default;

	bool operator==(const context_state_key &k) const {
		return name == k.name && index0 == k.index0 && index1 == k.index1;
	}
	bool operator!=(const context_state_key &k) const {
		return !((*this) == k);
	}

	bool operator<(const context_state_key &k) const {
		if (name != k.name)
			return name < k.name;
		if (index0 != k.index0)
			return index0 < k.index0;
		return index1 < k.index1;
	}

	auto &get_name() const { return name; }
	auto get_index0() const { return index0; }
	auto get_index1() const { return index1; }
};

}
}
}


namespace std {

template <typename K>
struct hash<StE::Core::GL::context_state_key<K>> {
	size_t inline operator()(const StE::Core::GL::context_state_key<K> &x) const {
		std::int64_t h1 = static_cast<std::int64_t>(x.index0) + (static_cast<std::int64_t>(x.index1) << 32);

		using T = typename std::underlying_type<decltype(x.name)>::type;

		auto n = static_cast<T>(x.name);
		return StE::hash_combine(std::hash<std::int64_t>()(h1), std::hash<T>()(n));
	}
};

}

