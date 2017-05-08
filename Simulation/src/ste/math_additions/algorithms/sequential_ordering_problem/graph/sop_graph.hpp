// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <graph.hpp>
#include <sop_vertex.hpp>
#include <sop_edge.hpp>

#include <is_base_of.hpp>
#include <functional>

// #include <graphviz/cgraph.h>
// #include <graphviz/gvc.h>

namespace ste {
namespace algorithm {
namespace SOP {

template <typename V, typename E>
class sop_graph : public graph::graph<V, E> {
	using Base = graph::graph<V, E>;

	static_assert(ste::is_base_of<V, sop_vertex>::value, "V must derive from algorithm::SOP::sop_vertex!");
	static_assert(std::is_base_of<sop_edge, E>::value, "E must derive from algorithm::SOP::sop_edge!");

public:
	virtual ~sop_graph() noexcept {}

	// bool write_trail_dot(const boost::filesystem::path &p, bool include_zero_trail_edges = false) const {
	// 	Agraph_t* g;
	// 	g = agopen("G", Agdirected, NULL);

	// 	agattr(g, AGEDGE, "label", "");

	// 	std::unordered_map<const graph::vertex*, Agnode_t*> vm;
	// 	for (auto vert : Base::get_vertices()) {
	// 		auto node = agnode(g, const_cast<char*>(vert->get_name().data()), true);
	// 		vm.insert(std::make_pair(vert, node));
	// 	}
	// 	for (auto e : Base::get_edges()) {
	// 		if (!include_zero_trail_edges && e->trail == E::t0)
	// 			continue;

	// 		Agnode_t* u = vm[e->get_from()];
	// 		Agnode_t* v = vm[e->get_to()];

	// 		auto ag_e = agedge(g, u, v, "", true);
	// 		auto label = std::to_string(e->trail) + "/" + std::to_string(e->get_weight());
	// 		agset(ag_e, "label", const_cast<char*>(label.data()));
	// 	}

	// 	bool ret = false;
	// 	FILE *f = fopen(p.string().data(), "wb");
	// 	if (f) {
	// 		agwrite(g, f);
	// 		fclose(f);

	// 		ret = true;
	// 	}
	// 	else
	// 		perror("graph_impl::write_dot - fopen() failed");

	// 	agclose(g);

	// 	return ret;
	// }
};

}
}
}
