
#include "stdafx.h"
#include "RenderControl.h"
#include "Log.h"

using namespace StE::LLR;

bool RenderControl::init_render_context(const char *title, int w, int h, bool fs) {
	context = std::unique_ptr<RenderContext>(new RenderContext());

	// create the window
	ste_log() << "Creating window " << w << "px x " << h << "px";
	this->window = context->create_window(title, w, h, fs);
	if (window == nullptr) {
		ste_log() << "window creation failed!";
		return false;
	}

	window->setMouseCursorVisible(false);

	set_projection_dirty();

	return true;
}

void RenderControl::run_loop(std::function<bool()> process) {
	bool running = true;
	while (running) {
		// handle events
		sf::Event event;
		while (this->window->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				// end the program 
				running = false;
			}
			else if (event.type == sf::Event::Resized) {
				// adjust the viewport when the window is resized 
				context->set_view_port(0, 0, event.size.width, event.size.height);
				set_projection_dirty();
			}
		}

		// draw...
		running &= process();

		// end the current frame (internally swaps the front and back buffers)
		this->window->display();
	}
}
