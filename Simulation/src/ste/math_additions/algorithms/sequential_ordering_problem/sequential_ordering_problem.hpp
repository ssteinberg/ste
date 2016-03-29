// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "sop_graph.hpp"

#include <vector>
#include <unordered_set>

#include <algorithm>
#include <functional>

namespace StE {
namespace Algorithm {
namespace SOP {
	
namespace detail {

class optimize_sequential_ordering_impl {
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
				assert(v->missing_deps == nullptr);
#endif
			}
		}
	};

public:
	template <typename V, typename E>
	using sequential_ordering_problem_solution = std::vector<std::pair<const V*, const E*>>;

public:
	template <typename V>
	static inline void generate_missing_deps(const V* node) {
		node->missing_deps = std::make_unique<V::DepsContainerT>(node->get_dependencies());
	}

	template <typename E, typename V>
	static inline auto next_pair_from_graph(const V* root) {
		for (auto &e : root->get_outgoing_edges()) {
			auto to = reinterpret_cast<const V*>(e->get_to());
			if (to->visited)
				continue;
			
			bool accept = false;
			if (to->missing_deps == nullptr) {
				if (to->get_dependencies().size() == 0)
					accept = true;
				else
					generate_missing_deps(to);
			}
			else
				accept = to->missing_deps->size() == 0; 
			
			if (accept) {
				auto edge = reinterpret_cast<const E*>(e.get());
				return std::make_pair(to, edge);
			}
		}
		
		return std::pair<const V*, const E*>(nullptr, nullptr);
	}

	template <typename V>
	static inline void update_nodes_deps(const V* new_root) {
		new_root->visited = true;
		new_root->missing_deps = nullptr;
		
		for (auto &v : new_root->get_requisite_for()) {
			assert(!v->visited);
			if (v->missing_deps == nullptr)
				generate_missing_deps(v);
				
			v->missing_deps->erase(new_root);
		}
	}

	template <typename V, typename E>
	static inline sequential_ordering_problem_solution<V, E> optimize_sequential_ordering(const sop_graph<V, E> &g, const V *root) {
		assert(g.get_vertices().size() && root && "Invalid paramters");
		assert(root->get_dependencies().size() == 0 && "root has dependencies");
		
		visited_nodes_guard<V, E> guard(g);
		
		sequential_ordering_problem_solution<V, E> order;
		auto node = root;
		
		root->visited = true;
		while (order.size() < g.get_vertices().size() - 1) {
			auto next_pair = next_pair_from_graph<E>(node);
			if (!next_pair.first) {
				assert(false && "Transition can't be made");
				return order;
			}
			
			node = next_pair.first;
			order.push_back(std::move(next_pair));
			
			update_nodes_deps(node);
		}
		
		root->visited = false;
		auto next_pair = next_pair_from_graph<E>(node);
		if (!next_pair.first) {
			assert(false && "Transition can't be made");
			return order;
		}
		order.push_back(std::move(next_pair));
		
		return order;
	}
};

}

template <typename V, typename E>
using sequential_ordering_problem_solution = detail::optimize_sequential_ordering_impl::sequential_ordering_problem_solution<V, E>;

template <typename V, typename E>
inline sequential_ordering_problem_solution<V, E> optimize_sequential_ordering(const sop_graph<V, E> &g, const V *root) {
	return detail::optimize_sequential_ordering_impl::optimize_sequential_ordering<V, E>(g, root);
}
	
}
}
}
