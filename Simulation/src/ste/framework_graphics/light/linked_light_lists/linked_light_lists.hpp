// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "resource.hpp"

#include "Texture2D.hpp"

#include "buffer_usage.hpp"
#include "ShaderStorageBuffer.hpp"
#include "AtomicCounterBufferObject.hpp"

#include "gl_current_context.hpp"

#include <memory>
#include <limits>

namespace StE {
namespace Graphics {

class linked_light_lists {
private:
	struct lll_element {
		glm::uvec2 data;
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t virt_size = 2147483648 / 2;

	using lll_type = Core::ShaderStorageBuffer<lll_element, usage>;

public:
	static constexpr int lll_image_res_multiplier = 8;

private:
	lll_type lll;
	Core::ShaderStorageBuffer<std::uint32_t> lll_counter;
	std::unique_ptr<Core::Texture2D> lll_heads;
	std::unique_ptr<Core::Texture2D> lll_low_detail_heads;

	glm::ivec2 size;

	static std::size_t virtual_lll_size() {
		return (virt_size / lll_type::page_size() / sizeof(lll_element) + 1) * lll_type::page_size();
	}

public:
	linked_light_lists(glm::ivec2 size) : lll(virtual_lll_size()), lll_counter(1) { resize(size); }

	void resize(glm::ivec2 size);
	auto& get_size() const { return size; }

	void clear() {
		std::uint32_t zero = 0;
		lll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
	}

	void bind_readwrite_lll_buffers() const;
	void bind_lll_buffer(bool low_detail = false) const;
};

}
}
