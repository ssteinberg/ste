// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

namespace StE {
namespace LLR {

class llr_resource_stub_allocator {
public:
	static bool is_valid(unsigned int id) { return !!id; }
	static int allocate() { return 0; }
	static void deallocate(unsigned int &id) { id = 0; }
};

class GenericResource {
protected:
	unsigned int id;

	GenericResource() {}
	~GenericResource() noexcept {}

public:
	virtual llr_resource_type resource_type() const = 0;

	int get_resource_id() const { return id; }
	virtual bool is_valid() const = 0;

	GenericResource(GenericResource &&res) : id(res.id) { res.id = 0; }
	GenericResource &operator=(GenericResource &&res) = delete;
	GenericResource(const GenericResource &res) = delete;
	GenericResource &operator=(const GenericResource &res) = delete;
};

template <class A>
class llr_resource : virtual public GenericResource {
protected:
	using Allocator = A;

	llr_resource() { this->id = Allocator::allocate(); }
	~llr_resource() noexcept { if (Allocator::is_valid(id)) Allocator::deallocate(id); }

public:
	llr_resource(llr_resource &&m) = default;
	llr_resource(const llr_resource &c) = delete;
	llr_resource& operator=(llr_resource &&m) = default;
	llr_resource& operator=(const llr_resource &c) = delete;

	bool is_valid() const { return Allocator::is_valid(id); }
};

}
}
