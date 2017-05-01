//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <command.hpp>

namespace ste {
namespace gl {

class task : public command {
public:
	virtual ~task() noexcept {}
};

}
}
