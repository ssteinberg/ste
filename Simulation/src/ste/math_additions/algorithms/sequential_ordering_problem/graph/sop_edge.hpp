// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_edge.hpp"

namespace StE {
namespace Algorithm {
namespace SOP {

class sop_edge : public Graph::edge {
	using Base = Graph::edge;

public:
	using Base::Base;
	virtual ~sop_edge() noexcept {}
};
	
}
}
}
