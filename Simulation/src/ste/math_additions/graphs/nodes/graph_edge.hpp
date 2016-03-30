// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"

#include <functional>

namespace StE {
namespace Graph {

template <typename V, typename E>
class graph;

class edge {
	template <typename V, typename E>
	friend class graph;
	
private:
	int weight;
	
	const vertex *from, *to;

public:
	edge(int w, const vertex *from, const vertex *to) : weight(w), from(from), to(to) {
		assert(from && "from must be a non-null vertex");
		assert(to && "to must be a non-null vertex");
	}
	virtual ~edge() noexcept {}
	edge(const edge &) = default;									
	edge(edge &&) = default;
	edge &operator=(const edge &) = default;									
	edge &operator=(edge &&) = default;
	
	void set_weight(int w) { weight = w; }
	auto get_weight() const { return weight; }
	
	auto get_from() const { return from; }
	auto get_to() const { return to; }
};

}
}
