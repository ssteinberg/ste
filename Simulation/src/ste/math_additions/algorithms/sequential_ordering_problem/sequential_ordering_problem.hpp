// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "sop_graph.hpp"

#include "optional.hpp"

#include "is_base_of.hpp"

#include <vector>

#include <algorithm>
#include <functional>
#include <random>

namespace StE {
namespace Algorithm {
namespace SOP {

template <typename GraphType>
class sequential_ordering_optimization {
	static_assert(StE::is_base_of<GraphType, sop_graph>::value, "GraphType must derive from Algorithm::SOP::sop_graph!");
	
private:
	using V = typename GraphType::vertex_type;
	using E = typename GraphType::edge_type;

private:
	template <typename V, typename E>
	class visited_nodes_guard {
		const sop_graph<V, E> &g;
		
	public:
		visited_nodes_guard(const sop_graph<V, E> &g) : g(g) {}
		~visited_nodes_guard() {
			for (auto &v : g.get_vertices()) {
				v->visited = false;
#ifdef DEBUG
				assert(!!v->missing_deps);
#endif
			}
		}
	};

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
	
	inline void add_trail(const E* e, float t) {
		e->trail = glm::mix(e->trail, t, trail_ro);
	}
	
	inline void consume_trail(const E* e) {
		e->trail = glm::mix(e->trail, E::t0, trail_theta);
	}

private:
	inline void generate_missing_deps(const V* node) {
		if (node->get_dependencies().size() > 0 && !node->missing_deps)
			node->missing_deps.emplace(node->get_dependencies());
	}

	void update_nodes_deps(const V* new_root) {
		new_root->visited = true;
		new_root->missing_deps = none;
		
		for (auto &v : new_root->get_requisite_for()) {
			assert(v == new_root && "Node depends on itself!");
			assert(!v->visited);
			
			generate_missing_deps(reinterpret_cast<const V*>(v));
			assert(!!v->missing_deps);

			v->missing_deps.get().erase(new_root);
		}
	}
	
	inline float edge_transition_weight(const E* e) {
		return e->desirability() * (e->trail + trail_epsilon);
	}
	
	auto collect_feasible_transitions(const V* root) {
		std::vector<const E*> useable_edges;
		useable_edges.reserve(root->get_outgoing_edges().size());
		
		for (auto &e : root->get_outgoing_edges()) {
			auto to = reinterpret_cast<const V*>(e->get_to());
			if (to->visited)
				continue;
			
			generate_missing_deps(to);
			
			if (to->get_dependencies().size() == 0 || to->missing_deps.get().size() == 0) {
				auto edge = reinterpret_cast<const E*>(e.get());
				useable_edges.push_back(edge);
			}
		}
		
		return useable_edges;
	}

	auto next_pair_from_graph(const V* root) {
		std::vector<const E*> useable_edges = collect_feasible_transitions(root);
		
		if (useable_edges.size() == 1) {
			const E* e = *useable_edges.begin();
			return std::make_pair(reinterpret_cast<const V*>(e->get_to()), e);
		}

		float total_weight = .0f;
		for (auto &e : useable_edges) {
			auto w = edge_transition_weight(e);
			assert(w >= .0f);
			total_weight += w;
		}

		float r = distribution(rand_gen) * total_weight;
		
		float accum = .0f;
		for (auto &e : useable_edges) {
			accum += edge_transition_weight(e);
			if (accum >= r) {
				auto to = reinterpret_cast<const V*>(e->get_to());
				return std::make_pair(to, e);
			}
		}
		
		assert(false && "Transition can't be made");
		return std::pair<const V*, const E*>(nullptr, nullptr);
	}
	
	void append_pair_to_solution(sequential_ordering_problem_solution &solution, typename sequential_ordering_problem_solution::route_type::value_type &&pair) {
		solution.length += pair.second->get_weight();
		consume_trail(pair.second);
		solution.route.push_back(std::move(pair));
	}

	sequential_ordering_problem_solution sop_iterate(const V *root) {
		assert(g.get_vertices().size() && root && "Invalid paramters");
		assert(root->get_dependencies().size() == 0 && "root has dependencies");
		
		visited_nodes_guard<V, E> guard(g);
		
		sequential_ordering_problem_solution solution;
		auto &order = solution.route;
		auto node = root;
		
		update_nodes_deps(node);
		
		while (order.size() < g.get_vertices().size() - 1) {
			auto next_pair = next_pair_from_graph(node);
			if (!next_pair.first)
				return solution;
			
			node = next_pair.first;
			update_nodes_deps(node);
			
			append_pair_to_solution(solution, std::move(next_pair));
		}
		
		// Connect last node to root to complete Hamiltonian cycle
		const E* last_edge = nullptr;
		for (auto &e : node->get_outgoing_edges()) {
			auto to = reinterpret_cast<const V*>(e->get_to());
			if (to == root) {
				last_edge = reinterpret_cast<const E*>(e.get());
				break;
			}
		}
		assert(last_edge && "Can not complete cycle");
		if (!last_edge)
			return solution;
		append_pair_to_solution(solution, std::make_pair(root, last_edge));
		
		return solution;
	}
	
	void update_trail_for_choosen_solution(const sequential_ordering_problem_solution &solution) {
		float trail = 1.f / static_cast<float>(solution.length);
		for (auto &p : solution.route) {
			add_trail(p.second, trail);
		}
	}
	
private:
	const GraphType &g;

	std::random_device rd;
	std::mt19937 rand_gen;
	std::uniform_real_distribution<> distribution;
	
	optional<sequential_ordering_problem_solution> best_solution;
	int no_improvements_counter{ 0 };
	
public:
	sequential_ordering_optimization(const GraphType &g) : g(g), rand_gen(rd()), distribution(.0f, 1.f) {}
	
	auto& operator()(const V *root, int iterations = 1) {
		assert(iterations > 0);
		
		bool has_improvement = false;
		for (int i = 0; i < iterations; ++i) {
			auto solution = sop_iterate(root);
			if (!best_solution || solution.length < best_solution.get().length) {
				best_solution = std::move(solution);				
				has_improvement = true;
			}
		}
		
		if (has_improvement) {
			update_trail_for_choosen_solution(best_solution.get());
			no_improvements_counter = 0;
		}
		else {
			++no_improvements_counter;
		}
		
		return best_solution.get();
	}
	
	auto get_no_improvements_counter() const {
		return no_improvements_counter;
	}
	
	void clear_best_solution() {
		best_solution = none;
	}
};
	
}
}
}
