// StE
// © Shlomi Steinberg, 2015-2017

#pragma once

#include <packed_ptr.hpp>

namespace ste {

template <typename Ptr>
class ref_count_ptr {
private:
	packed_ptr<Ptr> d;

public:
	ref_count_ptr() = default;
	ref_count_ptr(std::uint16_t counter, Ptr ptr)
		: d(counter, ptr)
	{}

	auto* get() const { return d.get(); }
	void set(Ptr ptr) { d.set(ptr); }

	auto get_counter() const { return d.get_integer(); }
	void set_counter(std::uint16_t c) { d.set_integer(c); }
	void inc() { set_counter(get_counter() + 1); }
	void dec() { set_counter(get_counter() - 1); }

	auto* operator->() { return get(); }
	const auto* operator->() const { return get(); }
	auto& operator*() { return *get(); }
	const auto& operator*() const { return *get(); }
};

}
