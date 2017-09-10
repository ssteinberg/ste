//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <lib/string.hpp>

namespace ste {
namespace gl {

class ste_queue_family {
private:
	std::uint32_t family;

public:
	ste_queue_family() = default;
	ste_queue_family(std::uint32_t family) : family(family) {}
	ste_queue_family(ste_queue_family&&) = default;
	ste_queue_family &operator=(ste_queue_family&&) = default;
	ste_queue_family(const ste_queue_family&) = default;
	ste_queue_family &operator=(const ste_queue_family&) = default;
	auto& operator=(std::uint32_t family) {
		this->family = family;
		return *this;
	}

	bool operator==(const ste_queue_family &o) const { return family == o.family; }
	bool operator!=(const ste_queue_family &o) const { return family != o.family; }
	bool operator<(const ste_queue_family &o) const { return family < o.family; }
	bool operator<=(const ste_queue_family &o) const { return family <= o.family; }
	bool operator>(const ste_queue_family &o) const { return family > o.family; }
	bool operator>=(const ste_queue_family &o) const { return family >= o.family; }

	explicit operator std::uint32_t() const { return family; }
};

}
}
