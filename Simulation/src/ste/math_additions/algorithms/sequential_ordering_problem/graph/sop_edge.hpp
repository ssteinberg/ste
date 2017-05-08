// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <graph_vertex.hpp>
#include <graph_edge.hpp>

namespace ste {
namespace algorithm {
namespace SOP {

template <typename V, typename E>
class sop_graph;
template <typename GraphType>
class sequential_ordering_problem;
template <typename G>
class sop_optimizer;

class sop_edge : public graph::edge {
	using Base = graph::edge;

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

protected:
	virtual void update_weight_and_transition() const = 0;
	using Base::set_weight;

public:
	sop_edge(const graph::vertex *from, const graph::vertex *to) : Base(1, from, to) {}
	virtual ~sop_edge() noexcept {}
	sop_edge(const sop_edge &) = default;
	sop_edge(sop_edge &&) = default;
	sop_edge &operator=(const sop_edge &) = default;
	sop_edge &operator=(sop_edge &&) = default;
};

}
}
}
