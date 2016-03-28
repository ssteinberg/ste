// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"

namespace StE {
namespace Algorithm {
namespace SOP {
	
namespace detail {
class optimize_sequential_ordering_impl;
}

template <typename DepsContainer>
class sop_vertex : public Graph::vertex {
	using Base = Graph::vertex;
	
	friend class detail::optimize_sequential_ordering_impl;
	
private:
	// For optimization
	using DepsContainerT = DepsContainer;
	
	mutable std::unique_ptr<DepsContainer> missing_deps{ nullptr };
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
