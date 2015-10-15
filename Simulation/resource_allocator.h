// StE
// © Shlomi Steinberg, 2015

#pragma once

namespace StE {
namespace LLR {

class generic_resource_allocator {
public:
	virtual bool is_valid(unsigned id) const { return !!id; }

	virtual unsigned allocate() = 0;
	virtual void deallocate(unsigned &id) { id = 0; }
};

template <typename ... Ts>
class generic_resource_immutable_storage_allocator : public generic_resource_allocator {
public:
	virtual void allocate_storage(unsigned id, Ts... args) = 0;
};

}
}
