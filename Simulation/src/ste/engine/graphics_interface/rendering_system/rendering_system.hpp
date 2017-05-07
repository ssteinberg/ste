//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <storage.hpp>
#include <fragment.hpp>

#include <boost/container/flat_map.hpp>
#include <memory>
#include <type_traits>
#include <utility>
#include <tuple_call.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <typename Fragment, int N, typename Map>
decltype(auto) rendering_system_find_storage(const Map &storages) {
	using Storage = typename Fragment::template consumed_storage<N>;

	auto tag = &Storage::tag;
	auto it = storages.find(tag);
	if (it == storages.end()) {
		// Not found
		throw std::runtime_error("Storage not found");
	}

	return reinterpret_cast<const Storage&>(*it->second);

}

template <typename Fragment>
struct rendering_system_create_storages_tuple {
	template <typename Map, std::size_t... Indices>
	decltype(auto) operator()(const Map &storages, std::index_sequence<Indices...>) {
		return std::make_tuple(rendering_system_find_storage<Fragment, Indices>(storages)...);
	}
};

template <typename Fragment, int N>
struct rendering_system_call_fragment_command_buffer {
	template <typename Map>
	auto operator()(Fragment &fragment, const Map &storages) {
		using indices = std::make_index_sequence<N>;
		auto tuple = rendering_system_create_storages_tuple<Fragment>()(storages, indices{});
		return tuple_call(&fragment, 
						  &Fragment::command_buffer,
						  tuple);
	}
};
template <typename Fragment>
struct rendering_system_call_fragment_command_buffer<Fragment, 0> {
	template <typename Map>
	auto operator()(Fragment &fragment, const Map &) {
		return fragment.command_buffer();
	}
};

}

class rendering_system {
private:
	using storage_tag = const void*;
	using storage_map_t = boost::container::flat_map<storage_tag, std::unique_ptr<storage_base>>;

protected:
	std::reference_wrapper<const ste_context> ctx;
	storage_map_t storages;

	/**
	 *	@brief	Helper method to call a fragment 'command_buffer()' method, passing the list of storages consumed by the fragment.
	 */
	template <typename Fragment>
	auto fragment_command_buffer(Fragment &fragment) {
		using caller = _internal::rendering_system_call_fragment_command_buffer<Fragment, Fragment::consumed_storages_count>;
		return caller()(fragment, storages);
	}

public:
	rendering_system(const ste_context &ctx) : ctx(ctx) {}
	virtual ~rendering_system() noexcept {}

	/**
	 *	@brief	Creates a storage class of type Storage and adds it to the rendering system.
	 */
	template <typename Storage, typename... Args>
	void create_storage(Args&&... args) {
		static_assert(std::is_base_of_v<storage_base, Storage>, "Storage type should inherit from class storage<Storage>");

		storage_tag tag = &Storage::tag;
		std::unique_ptr<storage_base> storage = std::make_unique<Storage>(std::forward<Args>(args)...);

		auto ret = storages.emplace(tag, std::move(storage));
		if (!ret.second) {
			// Storage type already exists
			throw std::runtime_error("Storage already exists");
		}
	}

	/**
	*	@brief	Removes a storage class of type Storage from the rendering system.
	*/
	template <typename Storage>
	void remove_storage() {
		static_assert(std::is_base_of_v<storage_base, Storage>, "Storage type should inherit from class storage<Storage>");

		storage_tag tag = &Storage::tag;
		storages.erase(tag);
	}

	virtual void render() = 0;
};

}
}
