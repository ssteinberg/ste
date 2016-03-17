// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "texture_sparse.hpp"
#include "Texture2D.hpp"

#include "PixelBufferObject.hpp"

#include <vector>
#include <unordered_map>
#include <functional>
#include <forward_list>

#include <gli/gli.hpp>

namespace StE {
namespace Graphics {

struct sbta_key {
	glm::ivec2 size;
	gli::format format;
};

bool inline operator==(const sbta_key &lhs, const sbta_key &rhs) {
	return lhs.size == rhs.size &&
		lhs.format == rhs.format;
}

}
}

namespace std {

template <> struct hash<StE::Graphics::sbta_key> {
	size_t inline operator()(const StE::Graphics::sbta_key &x) const {
		std::uint64_t s = x.size.x + (static_cast<std::uint64_t>(x.size.y) << 32);
		std::uint32_t h1 = std::hash<decltype(s)>()(s);
		std::uint32_t h2 = static_cast<std::uint32_t>(x.format);
		std::uint64_t h = h1 + (static_cast<std::uint64_t>(h2) << 32);
		return std::hash<decltype(h)>()(h);
	}
};

}

namespace StE {
namespace Graphics {

class texture_pool {
private:
	struct sbta_value {
		std::vector<glm::ivec3> virtual_page_sizes;
		Core::texture_sparse_2d_array pool;

		std::forward_list<int> unclaimed_layers;
	};

public:
	struct sbta_handle {
		sbta_key properties;
		std::uint64_t pool_handle;
		int layer;
		int levels;
	};

private:
	friend class std::hash<sbta_key>;

	sbta_value create_new_pool(const sbta_key &key) {
		auto sizes = Core::texture_sparse_2d_array::page_sizes(key.format);
		auto tile_size = sizes[0];

		int levels = Core::texture_sparse_2d_array::calculate_mipmap_max_level(key.size);
		return sbta_value{ sizes, Core::texture_sparse_2d_array(key.format, { key.size.xy, Core::texture_sparse_2d_array::max_layers() }, levels, tile_size, 0), { static_cast<int>(0) } };
	}

protected:
	std::unordered_map<sbta_key, sbta_value> sbta;

public:
	texture_pool() {}
	~texture_pool() {}

	sbta_handle commit(const Core::Texture2D &tex) {
		sbta_key key = { tex.get_image_size(), tex.get_format() };
		decltype(sbta)::iterator it = sbta.find(key);
		if (it == sbta.end()) {
			it = sbta.insert(std::make_pair(key, create_new_pool(key))).first;
		}

		auto &v = it->second;
		auto next_free_it = v.unclaimed_layers.begin();
		auto layer = *next_free_it;
		if ((++next_free_it) == v.unclaimed_layers.end())
			++*v.unclaimed_layers.begin();
		else
			v.unclaimed_layers.erase_after(v.unclaimed_layers.before_begin());

		//v.pool.make_resident();
		for (int l = 0; l < tex.get_levels(); ++l) {
			v.pool.commit_tiles({ 0, 0, layer }, { key.size.x >> l, key.size.y >> l, 1 }, l);

			Core::PixelBufferObject<char> pbo(tex.get_storage_size(l));
			pbo.pack_from(tex, l, 0);
			pbo.unpack_to(v.pool, l, layer);
		}

		sbta_handle handle = { key, v.pool.get_texture_handle(), layer, tex.get_levels() };
		return handle;
	}

	void uncommit(const sbta_handle &handle) {
		decltype(sbta)::iterator it = sbta.find(handle.properties);
		if (it == sbta.end())
			return;
		auto &v = it->second;

		auto free_it = v.unclaimed_layers.begin();
		if (*free_it < handle.layer) {
			auto next_free_it = free_it;
			++free_it;
			while (next_free_it != v.unclaimed_layers.end() && *next_free_it < handle.layer) {
				free_it = next_free_it;
				next_free_it++;
			}
			v.unclaimed_layers.insert_after(free_it, handle.layer);
		}
		else
			v.unclaimed_layers.insert_after(v.unclaimed_layers.before_begin(), handle.layer);

		for (int l = 0; l < handle.levels; ++l)
			v.pool.uncommit_tiles({ 0, 0, handle.layer }, { handle.properties.size.x >> l, handle.properties.size.y >> l, 1 }, l);
	}
};

}
}
