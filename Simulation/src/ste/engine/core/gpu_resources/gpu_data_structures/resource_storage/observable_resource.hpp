// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "resource_storage_base.hpp"

namespace StE {
namespace Core {

template <typename, typename, template<class> typename>
class resource_storage;

/**
 *	@brief	Resource consumed by a resource_storage
 *
 *	@param Descriptor	Descriptor type. Descriptor is a POD that will be uploaded to the GPU by the resource_storage.
 */
template <typename Descriptor>
class observable_resource {
	template <typename, typename, template<class> typename>
	friend class resource_storage;
	friend class resource_storage_base<Descriptor>;

public:
	using resource_descriptor_type = Descriptor;
	using storage_type = resource_storage_base<resource_descriptor_type>;

private:
	std::size_t resource_storage_identifier;
	mutable storage_type *storage_ptr{ nullptr };

public:
	virtual ~observable_resource() {
		dealloc();
	}
	virtual const resource_descriptor_type &get_descriptor() const = 0;

protected:
	/**
	*	@brief	On data change that needs to be reflected on the GPU-side, call notify to schedule update.
	*/
	void notify();

public:
	/**
	*	@brief	Deallocate resource. Identical to calling resource's' storage erase_resource().
	*/
	void dealloc();

	bool is_valid() const { return storage_ptr != nullptr; }
};

}
}

#include "observable_resource_impl.hpp"
