
#include "stdafx.h"
#include "RenderContext.h"

using namespace StE::LLR;

std::unique_ptr<sf::Window> RenderContext::create_window(const char *title, int w, int h, bool fs) {
	auto window = std::unique_ptr<sf::Window>(new sf::Window(sf::VideoMode(w, h), title, fs ? sf::Style::Fullscreen : sf::Style::Default, sf::ContextSettings(32)));
	window->setVerticalSyncEnabled(true);

	glewInit();
	set_view_port(0, 0, w, h);
	enable_depth_test();

	return window;
}
