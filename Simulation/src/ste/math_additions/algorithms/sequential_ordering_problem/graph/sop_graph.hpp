// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph.hpp"
#include "sop_vertex.hpp"
#include "sop_edge.hpp"

#include "is_base_of.hpp"
#include <functional>

namespace StE {
namespace Algorithm {
namespace SOP {

template <typename V, typename E>
class sop_graph : public Graph::graph<V, E> {
	static_assert(StE::is_base_of<V, sop_vertex>::value, "V must derive from Algorithm::SOP::sop_vertex!");
	static_assert(std::is_base_of<sop_edge, E>::value, "E must derive from Algorithm::SOP::sop_edge!");
	
public:
	virtual ~sop_graph() noexcept {}
};

}
}
}
