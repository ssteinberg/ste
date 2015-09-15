// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <SFML/Window/Mouse.hpp>
#include <glm/glm.hpp>

namespace StE {
namespace HID {

class PointerInput {
public:
	enum class B {
		Unknown = -1,
		Left = 0,
		Right,
		Middle,
		A,
		B,
		ButtonCount
	};

	static bool is_button_pressed(const B &b) {
		switch (b) {
		case B::Left:
			return sf::Mouse::isButtonPressed(sf::Mouse::Left);
		case B::Right:
			return sf::Mouse::isButtonPressed(sf::Mouse::Right);
		case B::Middle:
			return sf::Mouse::isButtonPressed(sf::Mouse::Middle);
		case B::A:
			return sf::Mouse::isButtonPressed(sf::Mouse::XButton1);
		case B::B:
			return sf::Mouse::isButtonPressed(sf::Mouse::XButton2);
		default: 
			return false;
		}
	}

	static glm::i32vec2 position() {
		auto pp = sf::Mouse::getPosition();
		return{ pp.x, pp.y };
	}

	static void set_position(const glm::i32vec2 &pos) {
		sf::Mouse::setPosition({ pos.x, pos.y });
	}
};

}
}
