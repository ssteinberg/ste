// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <functional>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderContext.h"
#include "Pointer.h"

namespace StE {
namespace LLR {

class RenderControl {
private:
	std::unique_ptr<RenderContext> context;

	mutable glm::mat4 projection;
	float field_of_view;
	float near_clip;
	float far_clip;

	bool projection_dirty;
	void set_projection_dirty() { projection_dirty = true; }

public:
	std::unique_ptr<sf::Window> window;
	RenderControl(RenderControl &&m) = delete;
	RenderControl(const RenderControl &c) = delete;
	RenderControl& operator=(RenderControl &&m) = delete;
	RenderControl& operator=(const RenderControl &c) = delete;

	RenderControl() : projection_dirty(true), field_of_view(M_PI_4), near_clip(.1), far_clip(1000) {}
	~RenderControl() {}

	bool init_render_context(const char *title, int w, int h, bool fs);
	bool init_render_context(const char *title, int w, int h) { return this->init_render_context(title, w, h, false); }

	const RenderContext &render_context() { return *context; }

	void set_fov(float rad) { field_of_view = rad; set_projection_dirty(); }
	void set_clipping_planes(float near_clip_distance, float far_clip_distance) { near_clip = near_clip_distance; far_clip = far_clip_distance; set_projection_dirty(); }

	void run_loop(std::function<bool()> process);

	bool window_active() const { return window->hasFocus(); }

	glm::i32vec2 window_position() const {
		auto v = window->getPosition();
		return{ v.x, v.y };
	}

	glm::i32vec2 viewport_size() const {
		auto v = window->getSize();
		return{ v.x, v.y };
	}

	glm::mat4 projection_matrix() const {
		if (projection_dirty) {
			auto vs = viewport_size();
			float aspect = vs.x / vs.y;
			projection = glm::perspective(field_of_view, aspect, near_clip, far_clip);
		}
		return projection;
	}

	// Pointer - get/set position relative to context window
	glm::i32vec2 get_relative_pointer_position() {
		auto v = window_position();
		auto pp = HID::PointerInput::position();
		pp -= decltype(pp)({ v.x, v.y });
		return{ pp.x, pp.y };
	}
	void set_relative_pointer_position(const glm::i32vec2 &pos) {
		auto v = window_position();
		v += decltype(v)({ pos.x, pos.y });
		HID::PointerInput::set_position({ v.x, v.y });
	}
};

}
}
