// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "signal.hpp"

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

public:
	using resource_descriptor_type = Descriptor;
	using signal_type = signal<const observable_resource<Descriptor>*>;

private:
	std::size_t resource_storage_identifier;
	signal_type resource_notify_signal;

public:
	virtual ~observable_resource() {}
	virtual const resource_descriptor_type &get_descriptor() const = 0;

protected:
	const signal_type &resource_signal() const { return resource_notify_signal; }

	/**
	*	@brief	On data change that needs to be reflected on the GPU-side, call notify to schedule update.
	*/
	void notify() { resource_notify_signal.emit(this); }
};

}
}
