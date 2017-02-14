// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <sop_graph.hpp>

#include <optional.hpp>

#include <is_base_of.hpp>

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
		using route_type = std::vector<const E*>;

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

private:
	void update_solution_length(int delta) {
		best_solution.get().length += delta;
	}

public:
	sequential_ordering_problem(const GraphType &g) : g(g), rand_gen(rd()), distribution(.0f, 1.f) {}

	optimizer_type optimizer(const V *root, int iterations = 1);

	bool run_solution(std::function<void(const E*)> &&pre_transition, std::function<void(const E*)> &&post_transition) {
		if (!best_solution)
			return false;

		int length_delta = 0;
		for (auto p : best_solution.get().route) {
			auto old_weight = p->get_weight();

			pre_transition(p);
			p->update_weight_and_transition();
			post_transition(p);

			auto new_weight = p->get_weight();

			length_delta += new_weight - old_weight;
		}
		update_solution_length(length_delta);

		return true;
	}

	bool run_solution() {
		return run_solution([](const E*){}, [](const E*){});
	}

	auto &get_solution() const {
		return best_solution;
	}

	void clear_best_solution() {
		best_solution = none;
	}
};

}
}
}

#include <sop_optimizer.hpp>

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
