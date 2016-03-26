// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"
#include "graph_edge.hpp"

#include <unordered_set>
#include <memory>

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

namespace StE {
namespace Graph {
namespace detail {
	
extern Agraph_t* create_graphviz_graph(const std::unordered_set<std::shared_ptr<const vertex>> *vertices, 
									   const std::unordered_set<std::shared_ptr<const edge>> *edges);
	
}
}
}