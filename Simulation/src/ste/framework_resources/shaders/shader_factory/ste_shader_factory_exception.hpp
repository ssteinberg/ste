// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <stdafx.hpp>

#include <stdexcept>

namespace StE {
namespace Resource {

class ste_shader_factory_exception : public std::exception {
	using Base = std::exception;

public:
	using Base::Base;
};

}
}
