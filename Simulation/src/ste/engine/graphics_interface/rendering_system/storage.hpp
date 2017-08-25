//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <anchored.hpp>

#include <alias.hpp>
#include <lib/alloc.hpp>

namespace ste {
namespace gl {

class storage_base : anchored {
	friend class rendering_system;

private:
	std::uint32_t references{ 0 };
	const void* tag{ 0 };
	alias<rendering_system> rs{ nullptr };

protected:
	storage_base() {}

public:
	virtual ~storage_base() noexcept;

	storage_base(const storage_base &) = delete;
	storage_base &operator=(const storage_base &) = delete;

private:
	friend void intrusive_ptr_add_ref(storage_base *ptr) {
		++ptr->references;
	}
	friend void intrusive_ptr_release(storage_base *ptr) {
		assert(ptr->references > 0);
		if (--ptr->references == 0) {
			lib::default_alloc<storage_base>::destroy(ptr);
		}
	}
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
