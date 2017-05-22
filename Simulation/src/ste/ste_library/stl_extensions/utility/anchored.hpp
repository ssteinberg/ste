//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

namespace ste {

class anchored {
public:
	anchored() noexcept = default;
	virtual ~anchored() noexcept {}
	anchored(anchored&&) noexcept = delete;
	anchored &operator=(anchored&&) noexcept = delete;
};

}
