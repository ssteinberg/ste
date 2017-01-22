// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "core_resource_type.hpp"

#include "thread_constants.hpp"

#include <functional>
#include <memory>
#include <type_traits>

namespace StE {
namespace Core {

class generic_resource {
public:
	using type = std::uint32_t;

protected:
	using generic_resource_shared_type = std::shared_ptr<type>;

	generic_resource_shared_type id;

	generic_resource() = default;
	virtual ~generic_resource() {}

public:
	virtual core_resource_type resource_type() const = 0;

	type get_resource_id() const { return *id; }
	virtual bool is_valid() const = 0;

	generic_resource(generic_resource &&res) {}
	generic_resource &operator=(generic_resource &&res) { return *this; }
	generic_resource(const generic_resource &res) = delete;
	generic_resource &operator=(const generic_resource &res) = delete;
};

}
}

#include "resource_allocator.hpp"

namespace StE {
namespace Core {

template <class Allocator>
class resource : virtual public generic_resource {
private:
	static_assert(std::is_base_of<generic_resource_allocator, Allocator>::value, "Allocator must derive from generic_resource_allocator");

private:
	template <class A2>
	friend class resource;

	using generic_resource::id;

protected:
	Allocator allocator;

	resource() {
		assert(is_main_thread() && "Must create Core::resource instance on main thread");

		type *res_id = new type(allocator.allocate());
		this->id = generic_resource_shared_type(res_id, [=](type *ptr) {
			Allocator::deallocate(*ptr);
			delete ptr;
		});
	}
	template <class A2>
	explicit resource(const resource<A2> &res) { id = res.id; }
	explicit resource(const resource &res) { id = res.id; }
	~resource() {
		assert(is_main_thread() && "Must destroy Core::resource instance on main thread");
	}

public:
	resource(resource &&m) { id = std::move(m.id); }
	resource& operator=(resource &&m) {
		id = std::move(m.id);
		return *this;
	}
	resource& operator=(const resource &c) = delete;

	bool is_valid() const { return Allocator::is_valid(*id); }
};

}
}
