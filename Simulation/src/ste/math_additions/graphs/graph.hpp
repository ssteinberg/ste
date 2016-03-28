// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"
#include "graph_edge.hpp"

#include <algorithm>
#include <functional>
#include <unordered_set>

#define BOOST_FILESYSTEM_NO_DEPRECATED 
#include <boost/filesystem.hpp>

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

namespace StE {
namespace Graph {
	
template <typename V, typename E>
class graph;

namespace detail {

template <typename T>
using GraphSet = std::unordered_set<T>; 
	
class graph_impl {
	template <typename V, typename E>
	friend class StE::Graph::graph;
	
private:
	using VertexPtr = const vertex*;
	using EdgePtr = const edge*; 

private:
	static bool write_dot(const GraphSet<VertexPtr> *vertices, 
						  const GraphSet<EdgePtr> *edges, 
						  const boost::filesystem::path &p);
	static bool write_png(const GraphSet<VertexPtr> *vertices, 
						  const GraphSet<EdgePtr> *edges, 
						  const boost::filesystem::path &p);
};

}

template <typename V, typename E>
class graph {
	static_assert(std::is_base_of<vertex, V>::value, "V must derive from Graph::vertex!");
	static_assert(std::is_base_of<edge, E>::value, "E must derive from Graph::edge!");
	
private:
	using VertexPtr = const V*;
	using EdgePtr = const E*; 
	
public:
	using VerticesSet = detail::GraphSet<VertexPtr>;
	using EdgesSet = detail::GraphSet<EdgePtr>; 
	
private:
	VerticesSet vertices;
	EdgesSet edges;
	
private:
	template <typename T>
	void erase_from(std::vector<std::unique_ptr<const T>> &vec, const T *what) const {
		auto it = std::find_if(vec.begin(), vec.end(), [&](const std::unique_ptr<const T> &o) { return o.get() == what; });
		if (it != vec.end())
			vec.erase(it);
	}
	template <typename T>
	void erase_from(std::vector<const T*> &vec, const T *what) const {
		auto it = std::find(vec.begin(), vec.end(), what);
		if (it != vec.end())
			vec.erase(it);
	}

public:
	graph() {}
	virtual ~graph() noexcept {}
	
	void add_vertex(const VertexPtr &v) {
		assert(v);
		vertices.insert(v);
	}
	void erase_vertex(const VertexPtr &v) {
		for (auto &e : v->originating_edges) {
			edges.erase(static_cast<EdgePtr>(e.get()));
			erase_from(e->to->terminating_edges, e.get());
		}
		for (auto &e : v->terminating_edges) {
			edges.erase(static_cast<EdgePtr>(e));
			erase_from(e->from->originating_edges, e);
		}
		
		vertices.erase(v);
		v->terminating_edges.clear();
		v->originating_edges.clear();
	}
	
	const E* add_edge(std::unique_ptr<const E> &&e) {
		auto e_ptr = e.get();
		
		assert(e_ptr);
		
		e_ptr->from->originating_edges.push_back(std::move(e));
		
		edges.insert(e_ptr);
		e_ptr->to->terminating_edges.push_back(e_ptr);
		
		return e_ptr;
	}
	void erase_edge(const EdgePtr &e) {
		edges.erase(e);
		erase_from(e->to->terminating_edges, static_cast<const edge*>(e));
		erase_from(e->from->originating_edges, static_cast<const edge*>(e));
	}
	
	void erase_all_vertex_edges(const VertexPtr &v) {
		{
			for (auto &e : v->terminating_edges) {
				edges.erase(static_cast<EdgePtr>(e));
				if (e->from != v)
					erase_from(e->from->originating_edges, e);
			}
		}
		v->terminating_edges.clear();
		
		{
			for (auto &e : v->originating_edges) {
				edges.erase(static_cast<EdgePtr>(e.get()));
				if (e->to != v)
					erase_from(e->to->terminating_edges, e.get());
			}
		}
		v->originating_edges.clear();
	}
	
	void clear() {
		for (auto &v : vertices) {
			v->from.clear();
			v->to.clear();
		}
		
		vertices.clear();
		edges.clear();
	}
	
	const auto &get_vertices() const { return vertices; }
	const auto &get_edges() const { return edges; }
	
	bool write_dot(const boost::filesystem::path &p) const {
		return detail::graph_impl::write_dot(reinterpret_cast<const detail::GraphSet<detail::graph_impl::VertexPtr> *>(&vertices),
											 reinterpret_cast<const detail::GraphSet<detail::graph_impl::EdgePtr> *>(&edges),
											 p);
	}
	bool write_png(const boost::filesystem::path &p) const {
		return detail::graph_impl::write_png(reinterpret_cast<const detail::GraphSet<detail::graph_impl::VertexPtr> *>(&vertices),
											 reinterpret_cast<const detail::GraphSet<detail::graph_impl::EdgePtr> *>(&edges),
											 p);
	}
};

}
}
