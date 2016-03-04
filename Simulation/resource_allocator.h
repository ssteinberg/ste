// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <string>

#include "Log.h"
#include "AttributedString.h"
#include "attrib.h"

namespace StE {
namespace LLR {

class generic_resource_allocator {
public:
	virtual unsigned allocate() = 0;
	static void deallocate(unsigned &id) { id = 0; }
	static bool is_valid(unsigned id) { return !!id; }
};

template <typename ... Ts>
class generic_resource_immutable_storage_allocator : public generic_resource_allocator {
	virtual void allocate_storage(unsigned id, Ts... args) = 0;
};

}
}
