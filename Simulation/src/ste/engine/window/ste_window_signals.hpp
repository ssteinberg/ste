//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <signal.hpp>

#include <hid.hpp>

namespace ste {

class ste_window;

class ste_window_signals {
public:
	using window_resize_signal_type = signal<glm::i32vec2>;

	using hid_pointer_button_signal_type = signal<hid::button, hid::status, hid::modifier_bits>;
	using hid_pointer_movement_signal_type = signal<glm::dvec2>;
	using hid_scroll_signal_type = signal<glm::dvec2>;
	using hid_keyboard_signal_type = signal<hid::key, hid::key_scancode, hid::status, hid::modifier_bits>;
	using hid_text_input_signal_type = signal<std::uint32_t>;

private:
	window_resize_signal_type framebuffer_resize_signal;

	hid_pointer_movement_signal_type hid_pointer_movement_signal;
	hid_pointer_button_signal_type hid_pointer_button_signal;
	hid_scroll_signal_type hid_scroll_signal;
	hid_keyboard_signal_type hid_keyboard_signal;
	hid_text_input_signal_type hid_text_input_signal;

public:
	ste_window_signals(const ste_window *win);

	auto &signal_window_resize() { return framebuffer_resize_signal; }

	auto &signal_pointer_movement() { return hid_pointer_movement_signal; }
	auto &signal_pointer_button() { return hid_pointer_button_signal; }
	auto &signal_scroll() { return hid_scroll_signal; }
	auto &signal_keyboard() { return hid_keyboard_signal; }
	auto &signal_text_input() { return hid_text_input_signal; }
};

}
