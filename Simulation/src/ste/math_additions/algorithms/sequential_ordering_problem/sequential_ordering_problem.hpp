// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "sop_graph.hpp"

#include "optional.hpp"

#include "is_base_of.hpp"

#include <vector>

#include <functional>
#include <random>

#include <memory>

namespace StE {
namespace Algorithm {
namespace SOP {
	
template <typename G>
class sop_optimizer;

template <typename GraphType>
class sequential_ordering_problem {
	static_assert(StE::is_base_of<GraphType, sop_graph>::value, "GraphType must derive from Algorithm::SOP::sop_graph!");
	
	friend class sop_optimizer<GraphType>;
	
public:
	using optimizer_type = sop_optimizer<GraphType>;
	
private:
	using V = typename GraphType::vertex_type;
	using E = typename GraphType::edge_type;
	
private:
	static void add_trail(const E* e, float t) {
		e->trail = glm::mix(e->trail, t, trail_ro);
	}
	
	static void consume_trail(const E* e) {
		e->trail = glm::mix(e->trail, E::t0, trail_theta);
	}

public:
	struct sequential_ordering_problem_solution {
		using route_type = std::vector<std::pair<const V*, const E*>>;
		 
		route_type route;
		int length{ 0 };
	};
	
private:
	static constexpr float trail_ro = .1f;
	static constexpr float trail_theta = .1f;
	static constexpr float trail_epsilon = 1.f;
	
private:
	const GraphType &g;

	std::random_device rd;
	std::mt19937 rand_gen;
	std::uniform_real_distribution<> distribution;
	
	optional<sequential_ordering_problem_solution> best_solution;
	std::atomic<int> no_improvements_counter{ 0 };
	
public:
	sequential_ordering_problem(const GraphType &g) : g(g), rand_gen(rd()), distribution(.0f, 1.f) {}
	
	optimizer_type optimizer(const V *root, int iterations = 1);
	
	auto &get_solution() const {
		return best_solution;
	}
	
	int get_no_improvements_counter() const {
		return no_improvements_counter;
	}
	
	void reset_no_improvements_counter() {
		no_improvements_counter = 0;;
	}
	
	void clear_best_solution() {
		no_improvements_counter = 0;
		best_solution = none;
	}
};
	
}
}
}

#include "sop_optimizer.hpp"

namespace StE {
namespace Algorithm {
namespace SOP {
	
template <typename GraphType>
typename sequential_ordering_problem<GraphType>::optimizer_type sequential_ordering_problem<GraphType>::optimizer(const V *root, int iterations) {
	assert(iterations > 0);
	return optimizer_type(*this, root, iterations);
}
	
}
}
}
