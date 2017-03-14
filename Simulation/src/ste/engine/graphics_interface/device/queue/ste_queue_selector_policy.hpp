//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_engine_exceptions.hpp>

#include <optional.hpp>
#include <utility>

namespace StE {
namespace GL {

// Match quality
// 0 - perfect match
// >0 - worse match
// none - no match
using ste_queue_selector_policy_match_quality = optional<std::uint32_t>;

struct ste_queue_selector_policy_strict {
	static ste_queue_selector_policy_match_quality matches(const ste_queue_descriptor descriptor,
															  const std::pair<ste_queue_type, std::uint32_t> &select) {
		// Match stricly only
		bool strict_match = descriptor.type == select.first &&
			descriptor.type_index == select.second;
		if (strict_match)
			return 0;
		return none;
	}
	static std::uint32_t select_on_no_match(const ste_queue_descriptors &) {
		throw ste_engine_exception("Can not find compatible queue");
	}
};

struct ste_queue_selector_policy_flexible {
	static ste_queue_selector_policy_match_quality matches(const ste_queue_descriptor descriptor,
															  const std::pair<ste_queue_type, std::uint32_t> &select) {
		// Match a compatible queue. Prefer a strict match, otherwise any type_index will do.
		if (descriptor.type == select.first &&
			descriptor.type_index == select.second)
			return 0;

		std::uint32_t match_quality = 0;
		auto select_type = select.first;
		while (descriptor.type != select_type) {
			auto decayed = ste_decay_queue_type(select_type);
			if (decayed == select_type) {
				// Not compatible
				return none;
			}
			// Reduce match quality for each decay
			++match_quality;
			select_type = decayed;
		}

		// Found an imprefect match
		return match_quality;
	}
	static std::uint32_t select_on_no_match(const ste_queue_descriptors &) {
		throw ste_engine_exception("Can not find compatible queue");
	}
};

}
}
