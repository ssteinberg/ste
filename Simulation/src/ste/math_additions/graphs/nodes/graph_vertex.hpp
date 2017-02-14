// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace StE {
namespace Graph {
	
template <typename V, typename E>
class graph;
class edge;

class vertex {
	template <typename V, typename E>
	friend class graph;

private:
	mutable std::vector<const edge*> terminating_edges;
	mutable std::vector<std::unique_ptr<const edge>> originating_edges;

public:	
	virtual ~vertex() noexcept;
	
	const auto &get_outgoing_edges() const { return originating_edges; }
	const auto &get_incoming_edges() const { return terminating_edges; }
	
	virtual std::string get_name() const = 0;
};

}
}
