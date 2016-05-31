// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

namespace StE {
namespace Resource {

/**
*	@brief	Defines getter for resource_instance<R>
*
*	Partial specializations can be defined to override default get()
*
 *	@param R	resource type
*/
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
