//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <lib/flat_set.hpp>
#include <anchored.hpp>

#include <mutex>

namespace ste {
namespace gl {

template <typename Descriptor>
class observable_resource;

template <typename Descriptor>
class resource_storage_base : anchored {
	friend class observable_resource<Descriptor>;

protected:
	using descriptor_type = Descriptor;
	using resource_type = observable_resource<descriptor_type>;

protected:
	lib::flat_set<const resource_type*> signalled_objects;
	lib::flat_set<const resource_type*> objects;

	mutable std::mutex objects_mutex;

private:
	/**
	*	@brief	Queues the resource for GPU descriptor update
	*/
	void notify_resource(const resource_type *res) {
		assert(res);

		std::unique_lock<std::mutex> l(objects_mutex);
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
