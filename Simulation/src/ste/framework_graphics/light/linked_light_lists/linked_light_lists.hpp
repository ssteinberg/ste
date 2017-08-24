// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <texture.hpp>
#include <stable_vector.hpp>
#include <command_recorder.hpp>

#include <alias.hpp>
#include <lib/vector.hpp>
#include <atomic>

namespace ste {
namespace graphics {

class linked_light_lists {
private:
	using lll_element = gl::std430<glm::uvec2>;
	using lll_type = gl::stable_vector<lll_element>;

	using lll_counter_element = gl::std430<std::uint32_t>;
	using lll_counter_type = gl::stable_vector<lll_counter_element>;

public:
	static constexpr unsigned lll_image_res_multiplier = 8;

private:
	alias<const ste_context> ctx;

	lll_type lll;
	lll_counter_type lll_counter;

	ste_resource<gl::texture<gl::image_type::image_2d>> lll_heads;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_size;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_low_detail_heads;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_low_detail_size;

	glm::uvec2 extent;
	std::atomic_flag up_to_date;

	void resize_internal(gl::command_recorder &recorder);

public:
	linked_light_lists(const ste_context &ctx,
					   const glm::uvec2 &extent);
	~linked_light_lists() noexcept {}

	linked_light_lists(linked_light_lists&&) = default;

	void resize(const glm::uvec2 &extent);
	auto& get_extent() const { return extent; }

	void clear(gl::command_recorder &recorder) {
		if (up_to_date.test_and_set(std::memory_order_acquire))
			resize_internal(recorder);

		lib::vector<lll_counter_element> zero = { lll_counter_element(std::make_tuple<std::uint32_t>(0)) };
		recorder << lll_counter.overwrite_cmd(0, zero);
	}

	auto& linked_light_lists_buffer() const { return lll; }
	auto& linked_light_lists_counter_buffer() const { return lll_counter; }

	auto& linked_light_lists_heads_map() const { return *lll_heads; }
	auto& linked_light_lists_size_map() const { return *lll_size; }
	auto& linked_light_lists_low_detail_heads_map() const { return *lll_low_detail_heads; }
	auto& linked_light_lists_low_detail_size_map() const { return *lll_low_detail_size; }
};

}
}
