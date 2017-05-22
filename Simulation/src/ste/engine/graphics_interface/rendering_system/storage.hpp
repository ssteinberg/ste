//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <anchored.hpp>

namespace ste {
namespace gl {

class storage_base : anchored {
protected:
	storage_base() {}

public:
	virtual ~storage_base() noexcept {}

	storage_base(const storage_base &) = delete;
	storage_base &operator=(const storage_base &) = delete;
};

/**
*	@brief	A rendering system storage stores resources used by fragments for rendering.
*/
template <class CRTP>
class storage : public storage_base {
public:
	static int tag;

protected:
	storage() = default;

public:
	virtual ~storage() noexcept {}
};

template <class CRTP>
int storage<CRTP>::tag = 0;

}
}
