// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include <functional>
#include <string>

namespace StE {
namespace Graph {

class vertex {
public:	
	virtual std::string get_name() const = 0;
};

}
}
