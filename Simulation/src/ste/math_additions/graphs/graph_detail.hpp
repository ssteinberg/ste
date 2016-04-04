// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph.hpp"

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

namespace StE {
namespace Graph {
namespace detail {
	
extern Agraph_t* create_graphviz_graph(const GraphSet<graph_impl::VertexPtr> *vertices, 
									   const GraphSet<graph_impl::EdgePtr> *edges);
	
}
}
}
