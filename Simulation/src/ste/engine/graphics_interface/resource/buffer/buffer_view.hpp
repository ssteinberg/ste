//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <device_buffer_base.hpp>
#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

class buffer_view
	: public allow_type_decay<buffer_view, device_buffer_base>
{
private:
	const device_buffer_base *buffer;
	std::uint64_t offset_elements;
	std::uint64_t range_elements;

public:
	buffer_view(const device_buffer_base &buffer,
				std::uint64_t offset,
				std::uint64_t elements)
		: buffer(&buffer),
		offset_elements(offset),
		range_elements(elements)
	{
		assert(offset + elements <= buffer.get_elements_count());
	}
	buffer_view(const device_buffer_base &buffer,
				std::uint64_t offset)
		: buffer(&buffer),
		offset_elements(offset),
		range_elements(buffer.get_elements_count() - offset)
	{}
	buffer_view(const device_buffer_base &buffer)
		: buffer(&buffer),
		offset_elements(0),
		range_elements(buffer.get_elements_count())
	{}

	~buffer_view() noexcept {}

	buffer_view(buffer_view&&) = default;
	buffer_view(const buffer_view&) = default;
	buffer_view &operator=(buffer_view&&) = default;
	buffer_view &operator=(const buffer_view&) = default;

	/**
	 *	@brief	Offset (element count) of buffer view
	 */
	auto offset() const { return offset_elements; }
	/**
	*	@brief	Offset in bytes of buffer view
	*/
	auto offset_bytes() const { return byte_t(offset() * buffer->get_element_size_bytes()); }
	/**
	*	@brief	Range (element count) of buffer view
	*/
	auto range() const { return range_elements; }
	/**
	*	@brief	Range in bytes of buffer view
	*/
	auto range_bytes() const { return byte_t(range() * buffer->get_element_size_bytes()); }

	auto& get() const { return *buffer; }
};

}
}
