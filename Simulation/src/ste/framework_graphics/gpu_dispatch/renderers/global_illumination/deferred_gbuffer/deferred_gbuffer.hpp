// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "resource.hpp"

#include "Texture2D.hpp"
#include "FramebufferObject.hpp"

#include "buffer_usage.hpp"
#include "ShaderStorageBuffer.hpp"
#include "AtomicCounterBufferObject.hpp"

#include "gl_current_context.hpp"

#include <memory>

namespace StE {
namespace Graphics {

class deferred_gbuffer {
private:
	struct g_buffer_element {
		glm::mat2x4 data;
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t virt_size = 2147483648;

	using gbuffer_type = Core::ShaderStorageBuffer<g_buffer_element, usage>;

private:
	std::unique_ptr<Core::Texture2D> depth_target;
	Core::FramebufferObject fbo;

	gbuffer_type gbuffer;
	Core::AtomicCounterBufferObject<> gbuffer_ll_counter;
	std::unique_ptr<Core::Texture2D> gbuffer_ll_heads;

	glm::ivec2 size;

	static std::size_t virtual_gbuffer_size() {
		return virt_size / gbuffer_type::page_size() / sizeof(g_buffer_element) * gbuffer_type::page_size();
	}

public:
	deferred_gbuffer(glm::ivec2 size) : gbuffer(virtual_gbuffer_size()), gbuffer_ll_counter(1) { resize(size); }

	void resize(glm::ivec2 size);
	auto& get_size() const { return size; }

	void clear() {
		std::uint32_t zero = 0;
		std::uint32_t end = 0xFFFFFFFF;
		gbuffer_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
		gbuffer_ll_heads->clear(&end);
	}

	void bind_gbuffer(bool readonly = true) const;

	auto* get_depth_target() const { return depth_target.get(); }
	auto* get_fbo() const { return &fbo; }
};

}
}
