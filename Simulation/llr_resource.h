// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

#include <functional>
#include <memory>

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
	using generic_resource_type = unsigned;
	using generic_resource_shared_type = std::shared_ptr<generic_resource_type>;

	generic_resource_shared_type id;

	GenericResource() {}
	virtual ~GenericResource() noexcept {}

public:
	virtual llr_resource_type resource_type() const = 0;

	generic_resource_type get_resource_id() const { return *id; }
	virtual bool is_valid() const = 0;

	GenericResource(GenericResource &&res) : id(std::move(res.id)) {}
	GenericResource &operator=(GenericResource &&res) = delete;
	GenericResource(const GenericResource &res) = delete;
	GenericResource &operator=(const GenericResource &res) = delete;
};

template <class A>
class llr_resource : virtual public GenericResource {
private:
	template <class A2>
	friend class llr_resource;

	using GenericResource::id;

protected:
	using Allocator = A;

	llr_resource() { 
		generic_resource_type *res_id = new generic_resource_type(Allocator::allocate());
		this->id = generic_resource_shared_type(res_id, [](generic_resource_type *ptr) {
			if (Allocator::is_valid(*ptr)) 
				Allocator::deallocate(*ptr);
		});
	}
	template <class A2>
	explicit llr_resource(const llr_resource<A2> &res) { id = res.id; }
	explicit llr_resource(const llr_resource &res) { id = res.id; }
	~llr_resource() noexcept {}

public:
	llr_resource(llr_resource &&m) = default;
	llr_resource& operator=(llr_resource &&m) = default;
	llr_resource& operator=(const llr_resource &c) = delete;

	bool is_valid() const { return Allocator::is_valid(*id); }
};

}
}
