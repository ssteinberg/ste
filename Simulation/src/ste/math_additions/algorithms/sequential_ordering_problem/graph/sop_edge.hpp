// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_edge.hpp"

namespace StE {
namespace Algorithm {
namespace SOP {

template <typename V, typename E>
class sop_graph;
template <typename GraphType>
class sequential_ordering_problem;
template <typename G>
class sop_optimizer;

class sop_edge : public Graph::edge {
	using Base = Graph::edge;

	template <typename V, typename E>
	friend class sop_graph;
	template <typename GraphType>
	friend class sequential_ordering_problem;
	template <typename G>
	friend class sop_optimizer;
	
	static constexpr float t0 = .0f;

private:
	// For optimization
	inline float desirability() const {
		return 1.f / static_cast<float>(Base::get_weight());
	}
	mutable float trail{ t0 };

public:
	using Base::Base;
	virtual ~sop_edge() noexcept {}
};
	
}
}
}
