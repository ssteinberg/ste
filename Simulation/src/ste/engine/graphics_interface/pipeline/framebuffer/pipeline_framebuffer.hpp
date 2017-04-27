//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <framebuffer_attachment.hpp>
#include <pipeline_layout_attachment_location.hpp>

#include <boost/container/flat_map.hpp>
#include <optional.hpp>

namespace ste {
namespace gl {

class pipeline_framebuffer {
private:
	using location_t = pipeline_layout_attachment_location;
	using attachments_map_t = boost::container::flat_map<location_t, optional<framebuffer_attachment>>;

private:
	attachments_map_t attachments;
	
public:
	pipeline_framebuffer() = default;
	pipeline_framebuffer(const std::initializer_list<attachments_map_t::value_type> &il) : attachments(il) {}

	auto& operator[](const location_t &location) { return attachments[location]; }
	template <typename... Ts>
	auto try_emplace(Ts&&... ts) {
		return attachments.try_emplace(std::forward<Ts>(ts)...);
	}
	void erase(const location_t &location) { attachments.erase(location); }
	void erase(attachments_map_t::const_iterator it) { attachments.erase(it); }

	void insert(attachments_map_t::const_iterator it, attachments_map_t::value_type &&val) {
		attachments.insert(it, std::move(val));
	}

	auto find(const location_t &location) const { return attachments.find(location); }

	auto& get() const { return attachments; }
	auto begin() const { return attachments.begin(); }
	auto end() const { return attachments.end(); }
	auto begin() { return attachments.begin(); }
	auto end() { return attachments.end(); }

	auto size() {
		return attachments.size();
	}
};

}
}
