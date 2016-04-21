// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"
#include "graph_vertex.hpp"
#include "graph_edge.hpp"

#include <algorithm>
#include <functional>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
#include <boost/container/flat_set.hpp>

namespace StE {
namespace Graph {

template <typename V, typename E>
class graph;

namespace detail {

template <typename T>
using GraphSet = boost::container::flat_set<T>;

class graph_impl {
	template <typename V, typename E>
	friend class StE::Graph::graph;

public:
	using VertexPtr = std::shared_ptr<const vertex>;
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

public:
	using vertex_type = V;
	using edge_type = E;

private:
	using VertexPtr = std::shared_ptr<const V>;
	using EdgePtr = const E*;

public:
	using VerticesSet = detail::GraphSet<VertexPtr>;
	using EdgesSet = detail::GraphSet<EdgePtr>;

private:
	VerticesSet vertices;
	EdgesSet edges;

private:
	template <typename T, typename P>
	void erase_from(std::vector<T> &vec, const P *what) const {
		auto it = std::lower_bound(vec.begin(), vec.end(), what, [](const T &o, const P *w) -> bool { return o.get() < w; });
		if (it != vec.end())
			vec.erase(it);
	}
	template <typename T>
	void erase_from(std::vector<const T*> &vec, const T *what) const {
		auto it = std::lower_bound(vec.begin(), vec.end(), what);
		if (it != vec.end())
			vec.erase(it);
	}

public:
	graph() {}
	virtual ~graph() noexcept {}

	void add_vertex(const VertexPtr &v) {
		assert(v.get());
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

		v->terminating_edges.clear();
		v->originating_edges.clear();
		vertices.erase(v);
	}

	const E* add_edge(std::unique_ptr<const E> &&e) {
		auto e_ptr = e.get();

		assert(e_ptr);

#ifdef DEBUG
		for (auto &e : e_ptr->from->originating_edges)
			assert(e->to != e_ptr->to && "Edge exists");
#endif

		auto originating_it = std::lower_bound(e_ptr->from->originating_edges.begin(),
											   e_ptr->from->originating_edges.end(),
											   e.get(),
											   [](const std::unique_ptr<const edge> &o, const edge *w) -> bool { return o.get() < w; });
		e_ptr->from->originating_edges.insert(originating_it, std::move(e));

		edges.insert(e_ptr);

		auto terminating_it = std::lower_bound(e_ptr->to->terminating_edges.begin(),
											   e_ptr->to->terminating_edges.end(),
											   e_ptr);
		e_ptr->to->terminating_edges.insert(terminating_it, e_ptr);

		return e_ptr;
	}
	void erase_edge(const V *from, const V *to) {
		for (auto &e : from->originating_edges) {
			if (e->to == to) {
				erase_edge(static_cast<EdgePtr>(e.get()));
				return;
			}
		}
	}
	void erase_edge(const EdgePtr &e) {
		edges.erase(e);
		erase_from(e->to->terminating_edges, static_cast<const edge*>(e));
		erase_from(e->from->originating_edges, static_cast<const edge*>(e));
	}

	void erase_all_vertex_edges(const V *v) {
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

		edges.clear();
		vertices.clear();
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
