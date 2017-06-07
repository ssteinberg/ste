
#include "stdafx.hpp"
#include "gl_utils.hpp"

#include "ste_engine_control.hpp"
#include "Log.hpp"

#include "texture_2d.hpp"
#include "surface_factory.hpp"

#include "reversed_perspective.hpp"

#include <chrono>
#include <thread>
#include <exception>
#include <stdexcept>

#include <chrono>
#include <ctime>

#include "boost_filesystem.hpp"

using namespace StE;

struct StE::ste_engine_control_impl {
	float field_of_view{ glm::quarter_pi<float>() };
	float near_clip{ 0.1f };

	float fps{ 0 };
	int frames{ 0 };
	float total_time{ 0 };
	std::chrono::time_point<std::chrono::steady_clock> last_frame_time{ std::chrono::steady_clock::now() };
};

ste_engine_control::ste_engine_control(std::unique_ptr<Core::GL::gl_context> &&ctx) : pimpl(std::make_unique<ste_engine_control_impl>()), context(std::move(ctx)), global_cache("Cache", 1024 * 1024 * 256) {
	std::set_unexpected([]() {
		ste_log_fatal() << "Unhandled exception" << std::endl;
		std::abort();
	});

	assert(context.get());
	if (context == nullptr)
		throw std::runtime_error("context == nullptr");

	glfwSetErrorCallback([](int err, const char* description) { ste_log_error() << "GLFW reported an error (" << err << "): " << description; });

	Core::GL::gl_utils::dump_gl_info(false);

	glfwSetWindowUserPointer(context->window.get(), this);

	setup_signals();
	set_projection_dirty();
}

ste_engine_control::~ste_engine_control() noexcept {
}

void ste_engine_control::setup_signals() {
	glfwSetFramebufferSizeCallback(context->window.get(), [](GLFWwindow* winptr, int w, int h) {
		ste_engine_control *ec_this = reinterpret_cast<ste_engine_control*>(glfwGetWindowUserPointer(winptr));
		ec_this->context->resize({ w, h });
		ec_this->set_projection_dirty();

		ec_this->framebuffer_resize_signal.emit({ w, h });
	});

	glfwSetCursorPosCallback(context->window.get(), [](GLFWwindow* winptr, double xpos, double ypos) {
		ste_engine_control *ec_this = reinterpret_cast<ste_engine_control*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_movement_signal.emit({ xpos, ypos });
	});

	glfwSetScrollCallback(context->window.get(), [](GLFWwindow* winptr, double xoffset, double yoffset) {
		ste_engine_control *ec_this = reinterpret_cast<ste_engine_control*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_scroll_signal.emit({ xoffset, yoffset });
	});

	glfwSetMouseButtonCallback(context->window.get(), [](GLFWwindow* winptr, int button, int action, int mods) {
		ste_engine_control *ec_this = reinterpret_cast<ste_engine_control*>(glfwGetWindowUserPointer(winptr));
		ec_this->hid_pointer_button_signal.emit(HID::pointer::convert_button(button), HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});

	glfwSetKeyCallback(context->window.get(), [](GLFWwindow* winptr, int key, int scancode, int action, int mods) {
		ste_engine_control *ec_this = reinterpret_cast<ste_engine_control*>(glfwGetWindowUserPointer(winptr));

		auto k = HID::keyboard::convert_key(key);
		ec_this->hid_keyboard_signal.emit(k, scancode, HID::convert_status(action), static_cast<HID::ModifierBits>(mods));
	});
}

void ste_engine_control::update_tpf() {
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

bool ste_engine_control::tick() {
	update_tpf();

	global_scheduler.tick();
	glfwPollEvents();

	glfwSwapBuffers(context->window.get());
	global_renderer->render_queue();

	return !glfwWindowShouldClose(context->window.get());
}

void ste_engine_control::capture_screenshot() const {
	auto size = gl()->framebuffer_size();

	auto fbo = std::make_unique<StE::Core::framebuffer_object>();
	StE::Core::texture_2d fbo_tex(gli::format::FORMAT_RGBA8_UNORM_PACK8, size, 1);
	(*fbo)[0] = fbo_tex[0];

	glFinish();
	gl()->defaut_framebuffer().blit_to(*fbo, size, size);
	gli::texture2d tex(gli::FORMAT_RGBA8_UNORM_PACK8, size);
	(*fbo)[0].read_pixels(tex.data(), 4 * size.x * size.y);

	for (int i=0; i<size.x * size.y; ++i)
		reinterpret_cast<glm::u8vec4*>(tex.data())[i].w = 255;

	scheduler().schedule_now([=]() {
		boost::filesystem::create_directory("Screenshots");

		time_t rawtime;
		struct tm * timeinfo;
		char buffer[256];
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 256, "%a %d%b%y %H.%M.%S", timeinfo);

		StE::Resource::surface_factory::write_surface_2d(tex, std::string("Screenshots/") + buffer + ".png");
	});
}

void ste_engine_control::set_fov(float rad) {
	pimpl->field_of_view = rad;
	set_projection_dirty();
}

void ste_engine_control::set_clipping_planes(float near_clip_distance) {
	pimpl->near_clip = near_clip_distance;
	set_projection_dirty();
}

void ste_engine_control::set_projection_dirty() {
	float aspect = get_projection_aspect();
	projection_change_signal.emit(aspect, pimpl->field_of_view, pimpl->near_clip);
}

float ste_engine_control::get_fov() const {
	return pimpl->field_of_view;
}

float ste_engine_control::get_near_clip() const {
	return pimpl->near_clip;
}

float ste_engine_control::get_projection_aspect() const {
	auto vs = get_backbuffer_size();
	return vs.y ? static_cast<float>(vs.x) / static_cast<float>(vs.y) : 1.f;
}
