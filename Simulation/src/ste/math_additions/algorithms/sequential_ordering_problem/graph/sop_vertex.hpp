// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"

#include "optional.hpp"

namespace StE {
namespace Algorithm {
namespace SOP {

template <typename V, typename E>
class sop_graph;
template <typename GraphType>
class sequential_ordering_optimization;

template <typename DepsContainer>
class sop_vertex : public Graph::vertex {
	using Base = Graph::vertex;
	
	template <typename V, typename E>
	friend class sop_graph;
	template <typename GraphType>
	friend class sequential_ordering_optimization;
	
private:
	// For optimization
	using DepsContainerT = DepsContainer;
	
	mutable optional<DepsContainer> missing_deps;
	mutable bool visited{ false };
	
public:
	using Base::Base;
	virtual ~sop_vertex() noexcept {}
	
	virtual const DepsContainer &get_dependencies() const = 0;
	virtual const DepsContainer &get_requisite_for() const = 0;
};
	
}
}
}
