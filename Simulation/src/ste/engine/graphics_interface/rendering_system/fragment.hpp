//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <command_recorder.hpp>
#include <ste_resource_traits.hpp>

#include <alias.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment defines a part of the rendering process.
*/
class fragment : ste_resource_deferred_create_trait {
public:
	using fragment_command_buffer_t = command_buffer_secondary<false>;

private:
	alias<const ste_context> ctx;

protected:
	fragment(const ste_context &ctx) : ctx(ctx) {}

public:
	virtual ~fragment() noexcept {}

	fragment(fragment&&) = default;
	fragment &operator=(fragment&&) = default;
	fragment(const fragment &) = delete;
	fragment &operator=(const fragment &) = delete;

	// Subclasses are expected to declare:
	//static lib::string name();

	/**
	*	@brief	Records the fragment's commands
	*/
	virtual void record(command_recorder &) = 0;

	auto& get_creating_context() const { return ctx.get(); }
	auto& device() const { return ctx.get().device(); }
};

inline auto &operator<<(command_recorder &recorder, fragment &frag) {
	frag.record(recorder);
	return recorder;
}

}
}
