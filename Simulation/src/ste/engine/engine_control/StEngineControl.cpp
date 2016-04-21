
#include "stdafx.hpp"
#include "gl_utils.hpp"

#include "StEngineControl.hpp"
#include "Log.hpp"

#include "Texture2D.hpp"
#include "SurfaceFactory.hpp"

#include <chrono>
#include <thread>
#include <exception>
#include <stdexcept>

#include <chrono>
#include <ctime>

#include <gli/gli.hpp>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

using namespace StE;

struct StE::ste_engine_control_impl {
	float field_of_view{ glm::quarter_pi<float>() };
	float near_clip{ 0.1 };
	float far_clip{ 1000 };

	float fps{ 0 };
	int frames{ 0 };
	float total_time{ 0 };
	std::chrono::time_point<std::chrono::steady_clock> last_frame_time{ std::chrono::steady_clock::now() };
};

StEngineControl::StEngineControl(std::unique_ptr<Core::GL::gl_context> &&ctx) : pimpl(std::make_unique<ste_engine_control_impl>()), context(std::move(ctx)), global_cache("Cache", 1024 * 1024 * 256) {
	assert(context.get());
	if (context == nullptr)
		throw std::runtime_error("context == nullptr");

	glfwSetErrorCallback([](int err, const char* description) { ste_log_error() << "GLFW reported an error (" << err << "): " << description; });

	Core::GL::gl_utils::dump_gl_info(false);

	glfwSetWindowUserPointer(context->window.get(), this);

	setup_signals();
	set_projection_dirty();
}

StEngineControl::~StEngineControl() noexcept {
}

void StEngineControl::setup_signals() {
	glfwSetFramebufferSizeCallback(context->window.get(), [](GLFWwindow* winptr, int w, int h) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->context->resize({ w, h });
		ec_this->set_projection_dirty();

		ec_this->framebuffer_resize_signal.emit({ w, h });
	});

	glfwSetCursorPosCallback(context->window.get(), [](GLFWwindow* winptr, double xpos, double ypos) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	glfwSetScrollCallback(context->window.get(), [](GLFWwindow* winptr, double xoffset, double yoffset) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	glfwSetMouseButtonCallback(context->window.get(), [](GLFWwindow* winptr, int button, int action, int mods) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_button_signal.emit(HID::pointer::convert_button(button), HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	glfwSetKeyCallback(context->window.get(), [](GLFWwindow* winptr, int key, int scancode, int action, int mods) {
		StEngineControl *ec_this = reinterpret_cast<StEngineControl*>(glfwGetWindowUserPointer(winptr));

		auto k = HID::keyboard::convert_key(key);
		ec_this->hid_keyboard_signal.emit(k, scancode, HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});
}

void StEngineControl::update_tpf() {
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> delta = now - pimpl->last_frame_time;
	tpf = delta;
	pimpl->last_frame_time = now;
	++pimpl->frames;
	if ((pimpl->total_time += delta.count()) > .5f) {
		pimpl->fps = static_cast<float>(pimpl->frames) / pimpl->total_time;
		pimpl->total_time = .0f;
		pimpl->frames = 0;
	}
}

bool StEngineControl::run_loop() {
	update_tpf();

	global_scheduler.run_loop();
	glfwPollEvents();

	glfwSwapBuffers(context->window.get());
	glFinish();
	global_renderer->render_queue();

	return !glfwWindowShouldClose(context->window.get());
}

void StEngineControl::capture_screenshot() const {
	auto size = gl()->framebuffer_size();

	auto fbo = std::make_unique<StE::Core::FramebufferObject>();
	StE::Core::Texture2D fbo_tex(gli::format::FORMAT_RGB8_UNORM_PACK8, size, 1);
	(*fbo)[0] = fbo_tex[0];

	gl()->defaut_framebuffer().blit_to(*fbo, size, size);
	gli::texture2d tex(gli::FORMAT_RGB8_UNORM_PACK8, size);
	(*fbo)[0].read_pixels(tex.data(), 3 * size.x * size.y);

	scheduler().schedule_now([=](optional<task_scheduler*> sched) {
		boost::filesystem::create_directory("Screenshots");

		time_t rawtime;
		struct tm * timeinfo;
		char buffer[256];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 256, "%a %d%b%y %H.%M.%S", timeinfo);

		StE::Resource::SurfaceFactory::write_surface_2d_task(tex, std::string("Screenshots/") + buffer + ".png")(sched);
	});
}

void StEngineControl::set_fov(float rad) {
	pimpl->field_of_view = rad;
	set_projection_dirty();
}

void StEngineControl::set_clipping_planes(float near_clip_distance, float far_clip_distance) {
	pimpl->near_clip = near_clip_distance;
	pimpl->far_clip = far_clip_distance;
	set_projection_dirty();
}

void StEngineControl::set_projection_dirty() {
	auto vs = get_backbuffer_size();
	float aspect = vs.y ? vs.x / vs.y : 1;
	projection = glm::perspective(pimpl->field_of_view, aspect, pimpl->near_clip, pimpl->far_clip);

	projection_change_signal.emit(projection, pimpl->field_of_view, pimpl->near_clip, pimpl->far_clip);
}

glm::mat4 StEngineControl::projection_matrix() const {
	return projection;
}

float StEngineControl::get_fov() const {
	return pimpl->field_of_view;
}

float StEngineControl::get_near_clip() const {
	return pimpl->near_clip;
}

float StEngineControl::get_far_clip() const {
	return pimpl->far_clip;
}
