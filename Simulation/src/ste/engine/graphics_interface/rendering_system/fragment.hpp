//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command_recorder.hpp>
#include <ste_resource_traits.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment defines a part of the rendering process.
*/
class fragment : ste_resource_deferred_create_trait {
public:
	using fragment_command_buffer_t = command_buffer_secondary<false>;

protected:
	fragment() {}

public:
	virtual ~fragment() noexcept {}

	fragment(fragment&&) = default;
	fragment &operator=(fragment&&) = default;
	fragment(const fragment &) = delete;
	fragment &operator=(const fragment &) = delete;

	// Subclasses are expected to declare:
	//static const std::string& name();

	/**
	*	@brief	Records the fragment's commands
	*/
	virtual void record(command_recorder &) = 0;
};

inline auto &operator<<(command_recorder &recorder, fragment &frag) {
	frag.record(recorder);
	return recorder;
}

}
}
