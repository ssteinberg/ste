// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"

#include <functional>

namespace StE {
namespace Graph {

class edge {
private:
	int weight;
	
	const vertex *from, *to;

public:
	edge(const edge &) = default;									
	edge(edge &&) = default;
	edge(int w, const vertex *from, const vertex *to) : weight(w),
														from(from),
														to(to) {}

	edge &operator=(const edge &) = default;									
	edge &operator=(edge &&) = default;
	
	void set_weight(int w) { weight = w; }
	auto get_weight() const { return weight; }
	
	auto get_from() const { return from; }
	auto get_to() const { return to; }
};

}
}
