// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "sequential_ordering_problem.hpp"

#include "task.hpp"

#include "Log.hpp"

#include <vector>
#include <algorithm>

#include <functional>

#include <atomic>

namespace StE {
namespace Algorithm {
namespace SOP {

template <typename G>
class sop_optimizer {
	friend class sequential_ordering_problem<G>;

private:
	using SOP = sequential_ordering_problem<G>;
	using V = typename G::vertex_type;
	using E = typename G::edge_type;

private:
	class visited_nodes_guard {
		const sop_graph<V, E> &g;

	public:
		visited_nodes_guard(const sop_graph<V, E> &g) : g(g) {}
		~visited_nodes_guard() {
			for (auto &v : g.get_vertices()) {
				v->visited = false;
#ifdef DEBUG
				assert(!v->missing_deps);
#endif
			}
		}
	};

private:
	inline void generate_missing_deps(const V* node) {
		if (node->get_dependencies().size() > 0 && !node->missing_deps) {
			node->missing_deps.emplace(V::MissingDepsContainerT());
			node->missing_deps.get().reserve(node->get_dependencies().size());
			for (auto &v : node->get_dependencies())
				node->missing_deps.get().insert(v.get());
		}
	}

	void update_nodes_deps(const V* new_root) {
		new_root->visited = true;
		new_root->missing_deps = none;

		for (auto &v : new_root->get_requisite_for()) {
			assert(v != new_root && "Node depends on itself!");
			assert(!v->visited);

			generate_missing_deps(reinterpret_cast<const V*>(v));
			assert(!!v->missing_deps);

			v->missing_deps.get().erase(new_root);
		}
	}

	inline float edge_transition_weight(const E* e) {
		return e->desirability() * (e->trail + SOP::trail_epsilon);
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

		float r = sop.distribution(sop.rand_gen) * total_weight;

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

	void add_edge_to_solution(typename SOP::sequential_ordering_problem_solution &solution, typename SOP::sequential_ordering_problem_solution::route_type::value_type &&edge) {
		solution.length += edge->get_weight();
		SOP::consume_trail(edge);
		solution.route.push_back(std::move(edge));
	}

	optional<typename SOP::sequential_ordering_problem_solution> sop_iterate(const V *root) {
		assert(sop.g.get_vertices().size() && root && "Invalid paramters");
		assert(root->get_dependencies().size() == 0 && "root has dependencies");

		visited_nodes_guard guard(sop.g);

		typename SOP::sequential_ordering_problem_solution solution;
		auto &order = solution.route;
		auto node = root;

		update_nodes_deps(node);

		while (order.size() < sop.g.get_vertices().size() - 1) {
			auto next_pair = next_pair_from_graph(node);
			if (!next_pair.first)
				return none;

			node = next_pair.first;
			update_nodes_deps(node);

			add_edge_to_solution(solution, std::move(next_pair.second));
		}

		if (node != root) {
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
				return none;
			add_edge_to_solution(solution, std::move(last_edge));
		}

		return solution;
	}

	void update_trail_for_choosen_solution(const typename SOP::sequential_ordering_problem_solution &solution) {
		float trail = 1.f / static_cast<float>(solution.length);
		for (auto p : solution.route) {
			SOP::add_trail(p, trail);
		}
	}

private:
	SOP &sop;
	const V * const root;
	const int iterations;

public:
	sop_optimizer() = delete;
	sop_optimizer(const sop_optimizer&) = delete;
	sop_optimizer &operator=(const sop_optimizer&) = delete;

	sop_optimizer(SOP &sop, const V *root, int iterations) : sop(sop), root(root), iterations(iterations) {}

	sop_optimizer(sop_optimizer&&) = default;
	sop_optimizer &operator=(sop_optimizer&&) = default;

public:
	void operator()() {
		optional<typename SOP::sequential_ordering_problem_solution> best_solution;
		for (int i = 0; i < iterations; ++i) {
			auto solution = sop_iterate(root);

			if (!solution)
				continue;
			if (!best_solution || solution.get().length < best_solution.get().length)
				best_solution = std::move(solution);
		}

		if (!sop.best_solution ||
			(best_solution && best_solution.get().length < sop.best_solution.get().length)) {
#ifdef DEBUG
			std::string path_str;
			for (auto p : best_solution.get().route)
				path_str += p->get_from()->get_name() + " -> ";
			ste_log() << "SOP - Found path of length " << std::to_string(best_solution.get().length) << ": " << path_str << std::endl;
#endif

			sop.best_solution = std::move(best_solution);

			update_trail_for_choosen_solution(sop.best_solution.get());
		}
	}
};

}
}
}
