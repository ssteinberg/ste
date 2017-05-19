//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>
#include <ste_queue_selector.hpp>

#include <algorithm>
#include <utility>
#include <lib/string.hpp>
#include <lib/flat_map.hpp>

namespace ste {
namespace gl {

class ste_device_queue_selector_cache {
private:
	using cache_key_t = lib::string;
	using cache_t = lib::flat_map<cache_key_t, std::uint32_t>;

public:
	ste_device_queue_selector_cache() = default;
	~ste_device_queue_selector_cache() noexcept {}

	template <typename selector_policy>
	auto operator()(const ste_queue_selector<selector_policy> &select,
					const ste_queue_descriptors &descriptors) const {
		static thread_local cache_t cached_usage_index_map;

		auto cache_key = cache_key_t(select.serialize_selector());
		{
			// Cached result. Return it.
			auto it = cached_usage_index_map.find(cache_key);
			if (it != cached_usage_index_map.end())
				return it->second;
		}

		// No cached result
		auto idx = select(descriptors);
		cached_usage_index_map.insert(std::make_pair(cache_key, idx));

		return idx;
	}
};

}
}
