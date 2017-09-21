//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <resource_storage_base.hpp>
#include <observable_resource.hpp>

#include <command.hpp>
#include <command_recorder.hpp>

#include <type_traits>
#include <lib/concurrent_queue.hpp>
#include <lib/unique_ptr.hpp>
#include <lib/deque.hpp>
#include <mutex>

namespace ste {
namespace gl {

/**
 *	@brief	resource_storage base class
 *
 *	resource_storage defines GPU storage of resources supporting updates and deletes.
 *
 *	@param Specialization	Subclass type (CRTP).
 *	@param Descriptor		Descriptor type. Descriptor is the POD that will be uploaded to the GPU for each resource.
 *	@param Storage			Storage class.
 */
template <typename Specialization, typename Descriptor, typename Storage>
class resource_storage : public resource_storage_base<Descriptor> {
	using Base = resource_storage_base<Descriptor>;

	friend Specialization;

protected:
	using resource_type = typename Base::resource_type;
	using descriptor_type = typename Base::descriptor_type;
	using storage_type = Storage;

protected:
	alias<const ste_context> ctx;

private:
	storage_type store;
	lib::concurrent_queue<command> pending_device_operations;

private:
	/**
	*	@brief	Used to allocate a new slot for a resource. Subclasses need to find space in the storage, upload the descriptor
	*			and return an unique identifier. The identifier is only meaningful to the subclass, and using the identifier
	*			the subclasses must be able to lookup the resource slot, even after insertion and/or deletion.
	*
	*	@param descriptor	New resource's descriptor to upload
	*/
	virtual lib::unique_ptr<command> allocate_identifier(const typename Base::descriptor_type &descriptor,
														 std::size_t &identifier_out) = 0;
	/**
	*	@brief	Notifies the subclass that the identifier is no longer required.
	*
	*	@param resource_storage_identifier	Identifier
	*/
	virtual lib::unique_ptr<command> deallocate_identifier(std::size_t resource_storage_identifier) = 0;
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
	lib::unique_ptr<T> allocate_resource(Ts&&... args) {
		static_assert(std::is_base_of<resource_type, T>::value, "T must derive from gl::observable_resource<Descriptor> !");

		// Craete new resource
		auto ptr = lib::allocate_unique<T>(std::forward<Ts>(args)...);
		ptr->storage_ptr = dynamic_cast<Specialization*>(this);

		// Find an identifier for the resource
		auto cmd = allocate_identifier(ptr->get_descriptor(),
									   ptr->resource_storage_identifier);

		// Insert into objects set
		{
			std::unique_lock<std::mutex> l(Base::objects_mutex);
			Base::objects.insert(ptr.get());
		}

		if (cmd != nullptr)
			pending_device_operations.push(std::move(cmd));

		return std::move(ptr);
	}

	/**
	*	@brief	Erase the resource, freeing a slot.
	*/
	virtual void erase_resource(const resource_type *res) override {
		assert(res);

		// Deallocate identifier
		auto cmd = deallocate_identifier(res->resource_storage_identifier);
		if (cmd != nullptr)
			pending_device_operations.push(std::move(cmd));

		// Remove from object sets
		{
			std::unique_lock<std::mutex> l(Base::objects_mutex);
			Base::objects.erase(res);
			Base::signalled_objects.erase(res);
		}

		res->storage_ptr = nullptr;
	}

public:
	resource_storage(const ste_context &ctx,
					 const buffer_usage &usage,
					 const char *name)
		: ctx(ctx),
		  store(ctx,
				usage, 
				name)
	{}
	virtual ~resource_storage() noexcept {
		std::unique_lock<std::mutex> l(Base::objects_mutex);
		for (auto &res : Base::objects)
			res->storage_ptr = nullptr;
	}

	/**
	*	@brief	Run an update pass, uploading descriptors of mutated resources. A resource is marked as mutated if its notify method
	*			was called.
	*/
	virtual void update(command_recorder &recorder) {
		// Pop and submit all pending operations
		lib::unique_ptr<command> pending_op = nullptr;
		while ((pending_op = pending_device_operations.pop()) != nullptr)
			recorder << std::move(*pending_op);

		// Likewise update all signalled objects
		{
			std::unique_lock<std::mutex> l(Base::objects_mutex);

			for (auto &res : Base::signalled_objects) {
				auto idx = index_of(res);
				recorder << store.overwrite_cmd(idx, res->get_descriptor());
			}

			Base::signalled_objects.clear();
		}
	}

	/**
	*	@brief	Get physical index of resource's slot in storage
	*/
	int index_of(const resource_type *res) const override final { return this->index_of_with_identifier(res->resource_storage_identifier); }

	/**
	*	@brief	Get underlying buffer
	*/
	const auto& buffer() const { return store; }

	/**
	*	@brief	Total count of active resources in storage
	*/
	std::size_t size() const {
		std::atomic_thread_fence(std::memory_order_acquire);
		return Base::objects.size();
	}
};

}
}

#include <observable_resource_impl.hpp>
