//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <resource_storage_base.hpp>

#include <alias.hpp>
#include <anchored.hpp>

namespace ste {
namespace gl {

template <typename, typename, typename>
class resource_storage;

/**
 *	@brief	Resource consumed by a resource_storage
 *
 *	@param Descriptor	Descriptor type. Descriptor is a POD that will be uploaded to the GPU by the resource_storage.
 */
template <typename Descriptor>
class observable_resource : anchored {
	template <typename, typename, typename>
	friend class resource_storage;
	friend class resource_storage_base<Descriptor>;

public:
	using resource_descriptor_type = Descriptor;
	using storage_type = resource_storage_base<resource_descriptor_type>;
	
private:
	std::size_t resource_storage_identifier;
	mutable alias<storage_type> storage_ptr{ nullptr };

public:
	virtual ~observable_resource() noexcept {
		dealloc();
	}
	virtual resource_descriptor_type get_descriptor() const = 0;

protected:
	/**
	*	@brief	On data change that needs to be reflected on the GPU-side, call notify to schedule update.
	*/
	void notify();

public:
	observable_resource() = default;

	observable_resource(const observable_resource&) = delete;
	observable_resource &operator=(const observable_resource&) = delete;

	/**
	*	@brief	Deallocate resource. Identical to calling resource's' storage erase_resource().
	*/
	void dealloc();

	auto resource_index_in_storage() const { return is_valid() ? storage_ptr->index_of(this) : -1; }
	bool is_valid() const { return storage_ptr != nullptr; }
};

}
}

#include <observable_resource_impl.hpp>
