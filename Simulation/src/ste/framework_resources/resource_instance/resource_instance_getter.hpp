// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

namespace StE {
namespace Resource {

template <typename R>
class resource_instance_getter {
public:
	auto &get(R *res) {
		return *res;
	}
	const auto &get(const R *res) {
		return *res;
	}
};

}
}
