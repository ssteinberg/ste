//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <pipeline_external_binding_set_collection.hpp>

#include <storage.hpp>
#include <storage_shared_ptr.hpp>

#include <boost/container/flat_set.hpp>
#include <type_traits>
#include <algorithm>

namespace ste {
namespace gl {

class rendering_system : ste_resource_deferred_create_trait {
private:
	using storage_tag = const void*;
	using storage_ptr_base = _internal::storage_shared_ptr_base<rendering_system>;
	using storage_map_t = boost::container::flat_set<storage_ptr_base>;

	friend void storage_ptr_base::release();

private:
	std::reference_wrapper<const ste_context> ctx;

protected:
	storage_map_t storages;

	/**
	*	@brief	Removes a storage class from the rendering system.
	*/
	void remove_storage(const storage_ptr_base &base) {
		storages.erase(base);
	}

public:
	rendering_system(const ste_context &ctx) : ctx(ctx) {}
	virtual ~rendering_system() noexcept {}

	/**
	*	@brief	Acquires a storage class of type Storage. If such a storage does not exist, it will be created.
	*			Returns a reference counting pointer to the new resource.
	*/
	template <typename Storage>
	auto acquire_storage() {
		static_assert(std::is_base_of_v<storage_base, Storage>, "Storage type should inherit from class storage<Storage>");

		storage_tag tag = &Storage::tag;
		// Find a storage with tag
		auto it = std::lower_bound(storages.begin(), storages.end(), tag, [](const storage_ptr_base &base, storage_tag tag) {
			return base.tag < tag;
		});
		if (it != storages.end() && it->tag == tag) {
			// We have a storage, return a shared pointer to it.
			return storage_shared_ptr<Storage>(*it);
		}

		// Need to create a storage
		storage_base* storage = new Storage(ctx.get());

		auto ret_it = storages.emplace_hint(it, storage, this, tag);
		return storage_shared_ptr<Storage>(*ret_it);
	}

	/**
	*	@brief	Implementations should use this to build a primary command queue and dispatch.
	*/
	virtual void render() = 0;

	auto& get_creating_context() const { return ctx.get(); }
	auto& device() const { return ctx.get().device(); }

	/**
	*	@brief	Implementations can use this hook to provide all fragments that use this rendering system with a collection of external binding sets.
	*/
	virtual const pipeline_external_binding_set_collection* external_binding_sets() const { return nullptr; }
};

}
}
