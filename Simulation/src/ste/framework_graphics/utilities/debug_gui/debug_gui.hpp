// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gpu_dispatchable.hpp>

#include <ste_engine_control.hpp>
#include <profiler.hpp>

#include <signal.hpp>
#include <font.hpp>

#include <camera.hpp>

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <functional>

namespace ste {
namespace Graphics {

class debug_gui : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using hid_pointer_button_signal_connection_type = ste_engine_control::hid_pointer_button_signal_type::connection_type;
	using hid_scroll_signal_connection_type = ste_engine_control::hid_scroll_signal_type::connection_type;
	using hid_keyboard_signal_connection_type = ste_engine_control::hid_keyboard_signal_type::connection_type;

private:
	const ste_engine_control &ctx;
	profiler *prof;
	const camera *cam;

	mutable std::map<std::string, std::vector<float>> prof_tasks_last_samples;

private:
	std::shared_ptr<hid_pointer_button_signal_connection_type> hid_pointer_button_signal;
	std::shared_ptr<hid_scroll_signal_connection_type> hid_scroll_signal;
	std::shared_ptr<hid_keyboard_signal_connection_type> hid_keyboard_signal;

	std::vector<std::function<void(const glm::ivec2 &)>> user_guis;

public:
	debug_gui(const ste_engine_control &ctx, profiler *prof, const ste::text::font &default_font, const camera *cam = nullptr);
	~debug_gui() noexcept;

	void add_custom_gui(std::function<void(const glm::ivec2 &)> &&f) { user_guis.emplace_back(std::move(f)); }

	bool is_gui_active() const;

protected:
	void set_context_state() const override final {}
	void dispatch() const override final;
};

}
}
