// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <string>

#include "resource.h"

#include "Log.h"
#include "AttributedString.h"
#include "attrib.h"

namespace StE {
namespace LLR {

class generic_resource_allocator {
public:
	virtual GenericResource::type allocate() = 0;
	static void deallocate(GenericResource::type &id) { id = 0; }
	static bool is_valid(GenericResource::type id) { return !!id; }
};

template <typename ... Ts>
class generic_resource_immutable_storage_allocator : public generic_resource_allocator {
	virtual void allocate_storage(GenericResource::type id, Ts... args) = 0;
};

}
}
