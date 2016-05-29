// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

#include "observable_resource.hpp"
#include "signal.hpp"

#include "is_base_of.hpp"

#include <type_traits>
#include <memory>

#include <unordered_set>
#include <map>

namespace StE {
namespace Core {

/**
 *	@brief	resource_storage base class
 *
 *	resource_storage defines GPU storage of resources supporting updates and deletes.
 *
 *	@param Specialization	Subclass type (CRTP).
 *	@param Descriptor		Descriptor type. Descriptor is the POD that will be uploaded to the GPU for each resource.
 *	@param Storage			Storage class.
 */
template <typename Specialization, typename Descriptor, template<class> typename Storage>
class resource_storage {
	friend Specialization;

	using descriptor_type = Descriptor;
	using resource_type = observable_resource<descriptor_type>;
	using storage_type = Storage<descriptor_type>;
	using resource_signal_connection_type = typename resource_type::signal_type::connection_type;

private:
	storage_type stack;

	std::unordered_set<const resource_type*> signalled_objects;
	std::unordered_map<const resource_type*, std::shared_ptr<resource_signal_connection_type>> connections;

private:
	/**
	*	@brief	Used to allocate a new slot for a resource. Subclasses need to find space in the storage, upload the descriptor
	*			and return an unique identifier. The identifier is only meaningful to the subclass, and using the identifier
	*			the subclasses must be able to lookup the resource slot, even after insertion and/or deletion.
	*
	*	@param descriptor	New resource's descriptor to upload
	*/
	virtual std::size_t allocate_identifier(const descriptor_type &descriptor) = 0;
	/**
	*	@brief	Notifies the subclass that the identifier is no longer required.
	*
	*	@param resource_storage_identifier	Identifier
	*/
	virtual void deallocate_identifier(std::size_t resource_storage_identifier) = 0;
	/**
	*	@brief	Overrides should return physical index based on identifier
	*
	*	@param resource_storage_identifier	Identifier
	*/
	virtual int index_of_with_identifier(std::size_t resource_storage_identifier) const = 0;

	void attach_resource_connection(resource_type *res) {
		auto connection = std::make_shared<resource_signal_connection_type>(
			[this](const resource_type* obj) { assert(obj); this->signalled_objects.insert(obj); }
		);
		res->resource_signal().connect(connection);
		connections.insert(std::make_pair(res, std::move(connection)));
	}

protected:
	/**
	*	@brief	Allocates a new slot, and returns a uniquely owned newly-constructed resource.
	*			At this point a defaultly constructed descriptor is already uploaded.
	*/
	template <typename T, typename ... Ts>
	std::unique_ptr<T> allocate_resource(Ts&&... args) {
		static_assert(std::is_base_of<resource_type, T>::value, "T must derive from Core::observable_resource<Descriptor> !");

		auto ptr = std::make_unique<T>(std::forward<Ts>(args)...);
		ptr->resource_storage_identifier = allocate_identifier(ptr->get_descriptor());
		attach_resource_connection(ptr.get());

		return std::move(ptr);
	}

	/**
	*	@brief	Erase the resource, freeing a slot.
	*/
	void erase_resource(const resource_type *res) {
		deallocate_identifier(res->resource_storage_identifier);
		connections.erase(res);
	}

public:
	virtual ~resource_storage() {}

	/**
	*	@brief	Run an update pass, uploading descriptors of mutated resources. A resource is marked mutated if its notify method
	*			was called.
	*/
	void update() {
		for (auto &res : signalled_objects) {
			auto idx = index_of(res);
			stack.overwrite(idx, res->get_descriptor());
		}

		signalled_objects.clear();
	}

	/**
	*	@brief	Get physical index of resource's slot in storage
	*/
	int index_of(const resource_type *res) const { return this->index_of_with_identifier(res->resource_storage_identifier); }

	/**
	*	@brief	Get SSBO
	*/
	auto &buffer() const { return stack.get_buffer(); }
};

}
}
