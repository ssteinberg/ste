// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <texture.hpp>
#include <array.hpp>
#include <command_recorder.hpp>

#include <alias.hpp>
#include <signal.hpp>

namespace ste {
namespace graphics {

class linked_light_lists {
private:
	static constexpr std::size_t max_lll_buffer_size = 1024 * 1024 * 1024;

	using lll_element = gl::std430<glm::uvec2>;
	using lll_type = gl::array<lll_element>;

	using lll_counter_element = gl::std430<std::uint32_t>;
	using lll_counter_type = gl::array<lll_counter_element>;

	mutable signal<> resize_signal;

public:
	static constexpr unsigned lll_image_res_multiplier = 8;

private:
	alias<const ste_context> ctx;

	glm::uvec2 extent;

	lll_type lll;
	lll_counter_type lll_counter;

	ste_resource<gl::texture<gl::image_type::image_2d>> lll_heads;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_size;

private:
	// Maximal size of LLL buffer
	std::size_t get_linked_light_list_required_size() const;

public:
	linked_light_lists(const ste_context &ctx,
					   const glm::uvec2 &extent);
	~linked_light_lists() noexcept {}

	linked_light_lists(linked_light_lists&&) = default;
	linked_light_lists &operator=(linked_light_lists&&) = default;

	void resize(const glm::uvec2 &extent);

	/*
	 *	@brief	Clears the LLL counter. Should be called every frame.
	 */
	void clear(gl::command_recorder &recorder) {
		const lll_counter_element zero = lll_counter_element(std::make_tuple<std::uint32_t>(0));
		recorder << lll_counter.overwrite_cmd(0, zero);
	}

	auto& linked_light_lists_buffer() const { return lll; }
	auto& linked_light_lists_counter_buffer() const { return lll_counter; }

	auto& linked_light_lists_heads_map() const { return *lll_heads; }
	auto& linked_light_lists_size_map() const { return *lll_size; }

	auto& get_extent() const { return extent; }
	auto& get_storage_modified_signal() const { return resize_signal; }
};

}
}
