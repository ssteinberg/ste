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

namespace ste {
namespace graphics {

class linked_light_lists {
private:
	using lll_element = gl::std430<glm::uvec2>;
	using lll_type = gl::stable_vector<lll_element>;

	using lll_counter_element = gl::std430<std::uint32_t>;
	using lll_counter_type = gl::stable_vector<lll_counter_element>;

public:
	static constexpr int lll_image_res_multiplier = 8;

private:
	alias<const ste_context> ctx;

	lll_type lll;
	lll_counter_type lll_counter;

	ste_resource<gl::texture<gl::image_type::image_2d>> lll_heads;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_size;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_low_detail_heads;
	ste_resource<gl::texture<gl::image_type::image_2d>> lll_low_detail_size;

	glm::ivec2 size;
	bool resize_on_clear{ false };

	void resize_internal(gl::command_recorder &recorder);

public:
	linked_light_lists(const ste_context &ctx, glm::ivec2 size);

	void resize(glm::ivec2 size) {
		size = size / lll_image_res_multiplier;
		if (size.x <= 0 || size.y <= 0 || size == this->size)
			return;

		resize_on_clear = true;
		this->size = size;
	}
	auto& get_size() const { return size; }

	void clear(gl::command_recorder &recorder) {
		if (resize_on_clear) {
			resize_internal(recorder);
			resize_on_clear = false;
		}

		lib::vector<lll_counter_element> zero = { lll_counter_element(std::make_tuple<std::uint32_t >(0)) };
		recorder << lll_counter.update_task(zero, 0)();
	}
};

}
}
