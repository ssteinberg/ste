// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <string>

#include "resource.hpp"

#include "Log.hpp"
#include "attributed_string.hpp"
#include "attrib.hpp"

namespace StE {
namespace Core {

class generic_resource_allocator {
public:
	virtual ~generic_resource_allocator() {}

	virtual generic_resource::type allocate() = 0;
	static void deallocate(generic_resource::type &id) { id = 0; }
	static bool is_valid(generic_resource::type id) { return !!id; }
};

template <typename ... Ts>
class generic_resource_immutable_storage_allocator : public generic_resource_allocator {
	virtual void allocate_storage(generic_resource::type id, Ts... args) = 0;
};

}
}
