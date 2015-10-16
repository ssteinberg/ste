// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "llr_resource_type.h"

#include "resource_allocator.h"

#include <functional>
#include <memory>
#include <type_traits>

namespace StE {
namespace LLR {

class GenericResource {
protected:
	using generic_resource_type = unsigned;
	using generic_resource_shared_type = std::shared_ptr<generic_resource_type>;

	generic_resource_shared_type id;

	GenericResource() = default;
	virtual ~GenericResource() {}

public:
	virtual llr_resource_type resource_type() const = 0;

	generic_resource_type get_resource_id() const { return *id; }
	virtual bool is_valid() const = 0;

	GenericResource(GenericResource &&res) : id(std::move(res.id)) {}
	GenericResource &operator=(GenericResource &&res) = delete;
	GenericResource(const GenericResource &res) = delete;
	GenericResource &operator=(const GenericResource &res) = delete;
};

template <class Allocator>
class resource : virtual public GenericResource {
private:
	static_assert(std::is_base_of<generic_resource_allocator, Allocator>::value, "Allocator must derive from generic_resource_allocator");

private:
	template <class A2>
	friend class resource;

	using GenericResource::id;

protected:
	Allocator allocator;

	resource() { 
		generic_resource_type *res_id = new generic_resource_type(allocator.allocate());
		this->id = generic_resource_shared_type(res_id, [=](generic_resource_type *ptr) {
			Allocator::deallocate(*ptr);
			delete ptr;
		});
	}
	template <class A2>
	explicit resource(const resource<A2> &res) { id = res.id; }
	explicit resource(const resource &res) { id = res.id; }
	~resource() {}

public:
	resource(resource &&m) = default;
	resource& operator=(resource &&m) = default;
	resource& operator=(const resource &c) = delete;

	bool is_valid() const { return Allocator::is_valid(*id); }
};

}
}
