// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "resource_storage_base.hpp"
#include "observable_resource.hpp"

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
class resource_storage : public resource_storage_base<Descriptor> {
	using Base = resource_storage_base<Descriptor>;

	friend Specialization;

protected:
	using resource_type = typename Base::resource_type;
	using descriptor_type = typename Base::descriptor_type;
	using storage_type = Storage<descriptor_type>;

private:
	storage_type stack;

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

protected:
	/**
	*	@brief	Allocates a new slot, and returns a uniquely owned newly-constructed resource.
	*			At this point a defaultly constructed descriptor is already uploaded.
	*/
	template <typename T, typename ... Ts>
	std::unique_ptr<T> allocate_resource(Ts&&... args) {
		static_assert(std::is_base_of<resource_type, T>::value, "T must derive from Core::observable_resource<Descriptor> !");

		auto ptr = std::make_unique<T>(std::forward<Ts>(args)...);
		ptr->storage_ptr = dynamic_cast<Specialization*>(this);
		ptr->resource_storage_identifier = allocate_identifier(ptr->get_descriptor());
		Base::objects.insert(ptr.get());

		return std::move(ptr);
	}

	/**
	*	@brief	Erase the resource, freeing a slot.
	*/
	virtual void erase_resource(const resource_type *res) override {
		assert(res);

		deallocate_identifier(res->resource_storage_identifier);
		Base::objects.erase(res);
		Base::signalled_objects.erase(res);

		res->storage_ptr = nullptr;
	}

public:
	virtual ~resource_storage() {
		for (auto &res : Base::objects)
			erase_resource(res);
	}

	/**
	*	@brief	Run an update pass, uploading descriptors of mutated resources. A resource is marked mutated if its notify method
	*			was called.
	*/
	void update() {
		for (auto &res : Base::signalled_objects) {
			auto idx = index_of(res);
			stack.overwrite(idx, res->get_descriptor());
		}

		Base::signalled_objects.clear();
	}

	/**
	*	@brief	Get physical index of resource's slot in storage
	*/
	int index_of(const resource_type *res) const override final { return this->index_of_with_identifier(res->resource_storage_identifier); }

	/**
	*	@brief	Get SSBO
	*/
	auto &buffer() const { return stack.get_buffer(); }

	/**
	*	@brief	Total count of active resources in storage
	*/
	std::size_t size() const { return Base::objects.size(); }
};

}
}

#include "observable_resource_impl.hpp"
