// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

namespace ste {

template <typename ... Params>
struct functor {
	virtual void operator()(Params&&... params) = 0;
	virtual ~functor() noexcept {}
};

}