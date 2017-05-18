//	StE
// © Shlomi Steinberg 2015-2016

#pragma once

#include <stdafx.hpp>
#include <ste_queue_type.hpp>
#include <ste_queue_selector_policy.hpp>
#include <ste_device_queue_descriptors.hpp>

namespace ste {
namespace gl {

using ste_queue_selector_default_policy = ste_queue_selector_policy_strict;

/**
*	@brief	Device queue selector
*/
template <typename selector_policy = ste_queue_selector_default_policy>
class ste_queue_selector {
private:
	using arg_type = typename selector_policy::queue_selector_arg_type;

private:
	arg_type arg;

public:
	template <typename... Args>
	ste_queue_selector(Args&&... args) : arg(std::forward<Args>(args)...) {}
	virtual ~ste_queue_selector() noexcept {}

	ste_queue_selector(ste_queue_selector&&) = default;
	ste_queue_selector &operator=(ste_queue_selector&&) = default;
	ste_queue_selector(const ste_queue_selector&) = default;
	ste_queue_selector &operator=(const ste_queue_selector&) = default;

	/**
	*	@brief	Device queue descriptor
	*
	*	@throws ste_engine_exception	If policy throws on no match
	*/
	std::uint32_t operator()(const ste_queue_descriptors &descriptors) const {
		// Find a match using the selecting policy.
		using match_t = std::pair<ste_queue_selector_policy_match_quality::type, ste_queue_descriptors::queues_t::const_iterator>;
		lib::vector<match_t> matches;
		for (auto it = descriptors.begin(); it != descriptors.end(); ++it) {
			auto match_result = selector_policy::matches(*it, this->arg);
			if (match_result) {
				matches.push_back(std::make_pair(match_result.get(), it));
				// Stop early
				if (match_result.get() == 0)
					break;
			}
		}

		// If none found. Ask policy what to do.
		if (!matches.size()) {
			return selector_policy::select_on_no_match(descriptors);
		}

		// We have matches. Select a match and return it
		auto match_it = std::min_element(matches.begin(), matches.end(), [](const auto &match1, const auto &match2) {
			return match1.first < match2.first;
		});
		assert(match_it != matches.end());
		auto it = match_it->second;
		assert(it != descriptors.end());

		std::uint32_t idx = static_cast<std::uint32_t>(it - descriptors.begin());
		return idx;
	}

	bool operator==(const ste_queue_selector &o) const {
		return arg == o.arg;
	}

	lib::string serialize_selector() const { return selector_policy::serialize_selector(arg); }
};

/**
*	@brief	Constructs a queue selector with default policy
*/
auto inline make_queue_selector(const ste_queue_type &type) {
	return ste_queue_selector<>(type);
}
/**
*	@brief	Constructs a queue selector with default policy
*/
auto inline make_queue_selector(const ste_queue_type &type,
								std::uint32_t type_index) {
	return ste_queue_selector<>(type, type_index);
}

/**
*	@brief	Constructs a queue selector with family policy
*/
auto inline make_family_queue_selector(const ste_queue_family &family) {
	return ste_queue_selector<ste_queue_selector_policy_family>(family);
}


/**
*	@brief	Helper method for a primary queue selector with default policy
*/
auto inline make_primary_queue_selector() {
	return make_queue_selector(ste_queue_type::primary_queue);
}
/**
*	@brief	Helper method for a graphics queue selector with default policy
*/
auto inline make_graphics_queue_selector() {
	return make_queue_selector(ste_queue_type::graphics_queue);
}
/**
*	@brief	Helper method for a compute queue selector with default policy
*/
auto inline make_compute_queue_selector() {
	return make_queue_selector(ste_queue_type::compute_queue);
}
/**
*	@brief	Helper method for a data transfer queue selector with default policy
*/
auto inline make_data_queue_selector() {
	return make_queue_selector(ste_queue_type::data_transfer_queue);
}
/**
*	@brief	Helper method for a sparse-binding queue selector with default policy
*/
auto inline make_sparse_binding_queue_selector() {
	return make_queue_selector(ste_queue_type::data_transfer_sparse_queue);
}

}
}
