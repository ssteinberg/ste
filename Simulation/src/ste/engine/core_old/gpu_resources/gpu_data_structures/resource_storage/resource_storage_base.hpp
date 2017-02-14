// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <stdafx.hpp>

#include <type_traits>
#include <memory>

#include <unordered_set>
#include <map>

namespace StE {
namespace Core {

template <typename Descriptor>
class observable_resource;

template <typename Descriptor>
class resource_storage_base {
	friend class observable_resource<Descriptor>;

protected:
	using descriptor_type = Descriptor;
	using resource_type = observable_resource<descriptor_type>;

protected:
	std::unordered_set<const resource_type*> signalled_objects;
	std::unordered_set<const resource_type*> objects;

private:
	/**
	*	@brief	Queues the resource for GPU descriptor update
	*/
	void notify_resource(const resource_type *res) {
		assert(res);
		signalled_objects.insert(res);
	}

	/**
	*	@brief	Get physical index of resource's slot in storage
	*/
	virtual int index_of(const resource_type *res) const = 0;

protected:
	/**
	*	@brief	Erase the resource, freeing a slot.
	*/
	virtual void erase_resource(const resource_type *res) = 0;

public:
	virtual ~resource_storage_base() {}
};

}
}
