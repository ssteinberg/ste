//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command_buffer.hpp>

#include <typelist.hpp>

namespace ste {
namespace gl {

/**
*	@brief	A rendering system fragment defines a part of the rendering process. Fragments work by generating a secondary command buffer for the rendering system to consume.
*/
template <typename... ConsumedStorages>
class fragment {
private:
	friend class rendering_system;

	static constexpr auto consumed_storages_count = sizeof...(ConsumedStorages);
	template <int N>
	using consumed_storage = typelist_type_at_t<N, ConsumedStorages...>;

public:
	using fragment_command_buffer_t = command_buffer_secondary<false>;

protected:
	fragment() {}

public:
	virtual ~fragment() noexcept {}

	fragment(const fragment &) = delete;
	fragment &operator=(const fragment &) = delete;

	// Subclasses are expected to declare:
	//static const std::string& name();

private:
	/**
	*	@brief	Generates the secondary command buffer used to render the fragment
	*/
	virtual fragment_command_buffer_t command_buffer(const ConsumedStorages&... storages) = 0;
};

/**
*	@brief	A rendering system fragment defines a part of the rendering process. Fragments work by generating a secondary command buffer for the rendering system to consume.
*/
template <>
class fragment<> {
private:
	friend class rendering_system;

	static constexpr auto consumed_storages_count = 0;

public:
	using fragment_command_buffer_t = command_buffer_secondary<false>;

protected:
	fragment() {}

public:
	virtual ~fragment() noexcept {}

	fragment(const fragment &) = delete;
	fragment &operator=(const fragment &) = delete;

	// Subclasses are expected to declare:
	//static const std::string& name();

private:
	/**
	*	@brief	Generates the secondary command buffer used to render the fragment
	*/
	virtual fragment_command_buffer_t command_buffer() = 0;
};

}
}
