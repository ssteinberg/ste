
#include "stdafx.hpp"
#include "graph.hpp"

#include "graph_detail.hpp"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

#include <map>

#include <iostream>

using namespace StE::Graph;
using namespace StE::Graph::detail;

Agraph_t* StE::Graph::detail::create_graphviz_graph(const GraphSet<graph_impl::VertexPtr> *vertices, 
													const GraphSet<graph_impl::EdgePtr> *edges) {
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

bool graph_impl::write_dot(const GraphSet<VertexPtr> *vertices, 
						   const GraphSet<EdgePtr> *edges, 
						   const boost::filesystem::path &p) {
	bool ret = false;
	
	Agraph_t* g = create_graphviz_graph(vertices, edges);
	
	FILE *f = fopen(p.string().data(), "wb");
	if (f) {
		agwrite(g, f);
		fclose(f);
		
		ret = true;
	}
	else 
		perror("graph_impl::write_dot - fopen() failed");
	
	agclose(g);
	
	return ret;
}

bool graph_impl::write_png(const GraphSet<VertexPtr> *vertices, 
						   const GraphSet<EdgePtr> *edges, 
						   const boost::filesystem::path &p) {
	Agraph_t* g = create_graphviz_graph(vertices, edges);
	
	GVC_t* gvc;
	gvc = gvContext();
	gvLayout(gvc, g, "dot");
	bool ret = gvRenderFilename(gvc, g, "dot", p.string().data()) == 0;
	gvFreeLayout(gvc, g);
	gvFreeContext(gvc);
	
	agclose(g);
	
	if (!ret)
		perror("graph_impl::write_png - gvRenderFilename() failed");
	
	return ret;
}
