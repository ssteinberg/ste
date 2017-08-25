//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <pipeline_external_binding_set_collection.hpp>

#include <storage.hpp>

#include <lib/flat_map.hpp>
#include <lib/intrusive_ptr.hpp>
#include <mutex>
#include <type_traits>
#include <algorithm>

#include <alias.hpp>
#include <anchored.hpp>

namespace ste {
namespace gl {

class rendering_system : ste_resource_deferred_create_trait, anchored {
public:
	template <typename Storage>
	using storage_ptr = lib::intrusive_ptr<Storage>;

private:
	using storage_tag = const void*;
	using storage_map_t = lib::flat_map<storage_tag, storage_base*>;

	friend class storage_base;

private:
	alias<const ste_context> ctx;
	std::mutex m;

protected:
	storage_map_t storages;

	/**
	*	@brief	Removes a storage class from the rendering system.
	*/
	void remove_storage(storage_tag tag) {
		std::unique_lock<std::mutex> l(m);
		storages.erase(tag);
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

		std::unique_lock<std::mutex> l(m);

		// Find a storage with tag
		auto it = std::lower_bound(storages.begin(), storages.end(), tag, [](const storage_map_t::value_type &pair, storage_tag tag) {
			return pair.second->tag < tag;
		});
		if (it != storages.end() && it->first == tag) {
			// We have a storage, return a shared pointer to it.
			return storage_ptr<Storage>(reinterpret_cast<Storage*>(it->second));
		}

		// Need to create a storage
		auto storage = lib::allocate_intrusive<Storage>(ctx.get());

		storage_base *base = storage.get();
		base->tag = tag;
		base->rs = this;
		storages.emplace_hint(it, std::make_pair(tag, base));

		return storage;
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
	virtual pipeline_external_binding_set_collection* external_binding_sets() const { return nullptr; }
};

}
}
