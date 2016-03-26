// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"
#include "graph_edge.hpp"

#include <functional>
#include <unordered_set>

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

#include <memory>

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

#include <map>

namespace StE {
namespace Graph {
	
template <typename V, typename E>
class graph;

namespace detail {
	
class graph_impl {
	template <typename V, typename E>
	friend class StE::Graph::graph;
	
private:
	using VertexPtr = std::shared_ptr<const vertex>;
	using EdgePtr = std::shared_ptr<const edge>;

private:
	static void write_dot(const std::unordered_set<VertexPtr> *vertices, 
						  const std::unordered_set<EdgePtr> *edges, 
						  const boost::filesystem::path &p);
	static void write_png(const std::unordered_set<VertexPtr> *vertices, 
						  const std::unordered_set<EdgePtr> *edges, 
						  const boost::filesystem::path &p);
};

}

template <typename V, typename E>
class graph {
	static_assert(std::is_base_of<vertex, V>::value, "V must derive from Graph::vertex!");
	static_assert(std::is_base_of<edge, E>::value, "E must derive from Graph::edge!");
	
private:
	using VertexPtr = std::shared_ptr<const V>;
	using EdgePtr = std::shared_ptr<const E>;
	
private:
	std::unordered_set<VertexPtr> vertices;
	std::unordered_set<EdgePtr> edges;

public:
	graph() {}
	
	void add_vertex(const VertexPtr &v) {
		vertices.insert(v);
	}
	
	void add_edge(const EdgePtr &e) {
		edges.insert(e);
	}
	
	void write_dot(const boost::filesystem::path &p) const {
		detail::graph_impl::write_dot(reinterpret_cast<const std::unordered_set<detail::graph_impl::VertexPtr> *>(&vertices),
									  reinterpret_cast<const std::unordered_set<detail::graph_impl::EdgePtr> *>(&edges),
									  p);
	}
	void write_png(const boost::filesystem::path &p) const {
		detail::graph_impl::write_png(reinterpret_cast<const std::unordered_set<detail::graph_impl::VertexPtr> *>(&vertices),
									  reinterpret_cast<const std::unordered_set<detail::graph_impl::EdgePtr> *>(&edges),
									  p);
	}
};

}
}
