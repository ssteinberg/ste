//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>
#include <ste_queue_family.hpp>
#include <ste_device_queue_descriptors.hpp>
#include <ste_engine_exceptions.hpp>

#include <optional.hpp>
#include <utility>
#include <string>

namespace ste {
namespace gl {

// Match quality
// 0 - perfect match
// >0 - worse match
// none - no match
using ste_queue_selector_policy_match_quality = optional<std::uint32_t>;

struct ste_queue_selector_policy_strict {
	struct arg_t {
		//	Queue type
		ste_queue_type type;
		// Queue type index
		std::uint32_t type_index{ 0 };

		arg_t() = default;
		arg_t(const ste_queue_type &type) : type(type) {}
		arg_t(const ste_queue_type &type, std::uint32_t type_index) : type(type), type_index(type_index) {}
	};

	using queue_selector_arg_type = arg_t;

	static ste_queue_selector_policy_match_quality matches(const ste_queue_descriptor descriptor,
														   const queue_selector_arg_type &select) {
		if (select.type == ste_queue_type::none ||
			select.type == ste_queue_type::all) {
			throw ste_engine_exception("ste_queue_type::none or ste_queue_type::all are not acceptable queue types");
		}

		// Match stricly only
		bool strict_match = descriptor.type == select.type &&
			descriptor.type_index == select.type_index;
		if (strict_match)
			return 0;
		return none;
	}
	static std::uint32_t select_on_no_match(const ste_queue_descriptors &) {
		throw ste_engine_exception("Can not find compatible queue");
	}

	static std::string serialize_selector(const queue_selector_arg_type &arg) {
		std::string data;
		data.resize(sizeof(arg));
		memcpy(data.data(), &arg, sizeof(arg));

		return data;
	}
};

struct ste_queue_selector_policy_flexible {
	using queue_selector_arg_type = ste_queue_selector_policy_strict::arg_t;

	static ste_queue_selector_policy_match_quality matches(const ste_queue_descriptor descriptor,
														   const queue_selector_arg_type &select) {
		if (select.type == ste_queue_type::none ||
			select.type == ste_queue_type::all) {
			throw ste_engine_exception("ste_queue_type::none or ste_queue_type::all are not acceptable queue types");
		}

		// Match a compatible queue. Prefer a strict match, otherwise any type_index will do.
		if (descriptor.type == select.type &&
			descriptor.type_index == select.type_index)
			return 0;

		std::uint32_t match_quality = 0;
		auto select_type = select.type;
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

	static std::string serialize_selector(const queue_selector_arg_type &arg) {
		return ste_queue_selector_policy_strict::serialize_selector(arg);
	}
};

struct ste_queue_selector_policy_family {
	using queue_selector_arg_type = ste_queue_family;

	static ste_queue_selector_policy_match_quality matches(const ste_queue_descriptor descriptor,
														   const queue_selector_arg_type &select) {
		if (descriptor.family != select)
			return none;

		return 0;
	}
	static std::uint32_t select_on_no_match(const ste_queue_descriptors &) {
		throw ste_engine_exception("Can not find compatible queue");
	}

	static std::string serialize_selector(const queue_selector_arg_type &arg) {
		std::string data;
		data.resize(sizeof(arg));
		memcpy(data.data(), &arg, sizeof(arg));

		return data;
	}
};

}
}
