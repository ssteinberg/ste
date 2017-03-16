//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

namespace StE {
namespace GL {

class ste_resource_pool_resetable_trait {
public:
	virtual ~ste_resource_pool_resetable_trait() noexcept {}
	virtual void reset() = 0;
};

}
}
