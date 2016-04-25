// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "resource.hpp"

#include "RenderTarget.hpp"
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
		glm::vec3 P;	std::uint32_t next_ptr;
		glm::tvec3<float, glm::mediump> N, T;	std::int16_t specular; std::uint16_t material;
		glm::tvec4<float, glm::mediump> albedo;
	};

	static constexpr Core::BufferUsage::buffer_usage usage = static_cast<Core::BufferUsage::buffer_usage>(Core::BufferUsage::BufferUsageSparse);
	static constexpr std::size_t pages = 2147483648;

	using gbuffer_type = Core::ShaderStorageBuffer<g_buffer_element, usage>;

private:
	std::unique_ptr<Core::RenderTarget> depth_target;
	Core::FramebufferObject fbo;

	gbuffer_type gbuffer;
	Core::AtomicCounterBufferObject<> gbuffer_ll_counter;
	std::unique_ptr<Core::Texture2D> gbuffer_ll_heads;

	glm::ivec2 size;

	static constexpr std::size_t virtual_gbuffer_size() {
		return pages / gbuffer_type::page_size() / sizeof(g_buffer_element) * gbuffer_type::page_size();
	}

public:
	deferred_gbuffer(glm::ivec2 size) : gbuffer(virtual_gbuffer_size()), gbuffer_ll_counter(1) { resize(size); }

	void resize(glm::ivec2 size);
	auto get_size() const { return size; }

	void clear() {
		std::uint32_t zero = 0;
		std::uint32_t end = 0xFFFFFFFF;
		gbuffer_ll_counter.clear(gli::FORMAT_R32_UINT_PACK32, &zero);
		gbuffer_ll_heads->clear(&end);
	}

	void bind_gbuffer() const;

	auto *get_fbo() const { return &fbo; }
};

}
}
