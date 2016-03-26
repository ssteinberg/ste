
#include "stdafx.hpp"
#include "graph.hpp"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

#include <map>

#include <iostream>

using namespace StE::Graph;
using namespace StE::Graph::detail;

Agraph_t* create_graphviz_graph(const std::unordered_set<std::shared_ptr<const vertex>> *vertices, 
								const std::unordered_set<std::shared_ptr<const edge>> *edges) {
	Agraph_t* g;
	g = agopen("G", Agdirected, NULL);
	
	agattr(g, AGEDGE, "label", "");
	
	std::unordered_map<const vertex*, Agnode_t*> vm;
	for (auto vert : *vertices) {
		auto node = agnode(g, const_cast<char*>(vert->get_name().data()), true);
		vm.insert(std::make_pair(vert.get(), node));
	}
	for (auto e : *edges) {
		Agnode_t* u = vm[e->get_from()];
		Agnode_t* v = vm[e->get_to()];
		
		auto ag_e = agedge(g, u, v, "", true);
		agset(ag_e, "label", const_cast<char*>(std::to_string(e->get_weight()).data()));
	}
	
	return g;
}

void graph_impl::write_dot(const std::unordered_set<VertexPtr> *vertices, 
						   const std::unordered_set<EdgePtr> *edges, 
						   const boost::filesystem::path &p) {
	Agraph_t* g = create_graphviz_graph(vertices, edges);
	
	FILE *f = fopen(p.string().data(), "wb");
	agwrite(g, f);
	fclose(f);
	
	agclose(g);
}

void graph_impl::write_png(const std::unordered_set<VertexPtr> *vertices, 
						   const std::unordered_set<EdgePtr> *edges, 
						   const boost::filesystem::path &p) {
	Agraph_t* g = create_graphviz_graph(vertices, edges);
	
	GVC_t* gvc;
	gvc = gvContext();
	gvLayout(gvc, g, "dot");
	gvRenderFilename(gvc, g, "dot", p.string().data());
	gvFreeLayout(gvc, g);
	gvFreeContext(gvc);
	
	agclose(g);
}
