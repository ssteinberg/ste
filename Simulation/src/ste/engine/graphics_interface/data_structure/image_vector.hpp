//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <ste_resource_traits.hpp>

#include <image.hpp>
#include <texture.hpp>
#include <pipeline_resource_binder.hpp>

#include <anchored.hpp>
#include <alias.hpp>

#include <lib/range_list.hpp>
#include <lib/vector.hpp>
#include <lib/shared_ptr.hpp>

#include <allow_type_decay.hpp>

namespace ste {
namespace gl {

/**
 *	@brief	image_vector (and storage_image_vector) are highly-specialized date structures that take ownership of gl::texture objects,
 *			and store an gl::pipeline::image alias of each texture in a vector that can later be used for binding a texture array
 *			into a pipeline.
 *			The vector is stable.
 */
template <image_type type, int dimensions = image_dimensions_v<type>, image_layout default_layout = gl::image_layout::shader_read_only_optimal>
class image_vector : anchored, ste_resource_deferred_create_trait {
private:
	using tombstone_ranges_t = lib::range_list<std::uint32_t>;
	using tombstone_range = tombstone_ranges_t::value_type;

	using texture_t = texture<type, dimensions>;
	using image_t = pipeline::image;

	using changes_vector_t = lib::vector<std::pair<std::uint32_t, pipeline::image>>;

public:
	class slot_t : public allow_type_decay<slot_t, texture_t> {
		friend class image_vector;
		struct token {};

	private:
		texture_t tex;

		alias<image_vector> storage;
		std::uint32_t slot_idx;

	public:
		slot_t(token,
			   texture_t &&tex,
			   image_vector &parent,
			   std::uint32_t slot_idx) 
			: tex(std::move(tex)),
			storage(parent),
			slot_idx(slot_idx) {}
		slot_t(slot_t&&) = default;
		slot_t(const slot_t&) = delete;
		slot_t &operator=(slot_t&&) = delete;
		slot_t &operator=(const slot_t&) = delete;
		~slot_t() noexcept {
			storage->erase(*this);
		}

		auto& get() const { return tex; }
		auto get_slot_idx() const { return slot_idx; }
	};

	using value_type = lib::shared_ptr<slot_t>;

private:
	void add_change(std::uint32_t idx, image_t &&img) {
		auto it = std::upper_bound(changes.begin(), changes.end(), idx, [](std::uint32_t idx, const auto &e) {
			return idx < e.first;
		});
		if (it != changes.end() && it->first == idx)
			it->second = std::move(img);
		else
			changes.insert(it, std::make_pair(idx, std::move(img)));
	}

	/**
	*	@brief	Marks an element with a tombstone. Tombstones can be consumed by calling insert(), overwriting the tombstone
	*			with new data.
	*/
	void erase(std::uint32_t idx) {
		assert(idx < size());

		// Set null handle image in slot
		v[idx] = image_t();

		// Add tombstone
		tombstone_range t(idx, 1);
		tombstones.add(t);

		// Update changes
		add_change(idx, image_t());
	}
	/**
	*	@brief	Marks an element with a tombstone. Tombstones can be consumed by calling insert(), overwriting the tombstone
	*			with new data.
	*/
	void erase(const slot_t &element) {
		erase(element.get_slot_idx());
	}

private:
	lib::vector<image_t> v;
	tombstone_ranges_t tombstones;

	mutable changes_vector_t changes;

public:
	image_vector() = default;
	~image_vector() noexcept {}

	image_vector(const image_vector&) = delete;
	image_vector &operator=(const image_vector&) = delete;

	/**
	*	@brief	Allocates a new slot for a texture and returns a shared pointer to the new slot.
	*			Slot lifetime is tied to the pointer's lifetime.
	*
	*	@param	tex		Texture to insert
	*/
	auto allocate_slot(texture_t &&tex,
					   image_layout layout = default_layout) {
		std::uint32_t location;

		// If there are tombstones, replace one of them with new element, if possible
		auto it = tombstones.begin();
		if (it != tombstones.end()) {
			location = it->start;
			tombstones.pop_front();
		}
		else {
			location = static_cast<std::uint32_t>(v.size());
		}

		// Create slot
		value_type val = lib::allocate_shared<slot_t>(slot_t::token(),
													  std::move(tex),
													  *this,
													  location);
		auto img = image_t(val->tex, layout);
		
		// Write image to vector and update changes data
		if (location < v.size()) {
			v[location] = img;
			add_change(location, std::move(img));
		}
		else {
			v.push_back(img);
			changes.push_back(std::make_pair(location, std::move(img)));
		}

		return val;
	}

	/**
	 *	@brief	Returns a vector of pipeline_resource_binder instances that bind the minimal amount of textures that changed since last 
	 *			call to binder(). 
	 *			Same return type as gl::bind, designed to be passed to a pipeline binding point.
	 */
	auto binder() const {
		lib::vector<std::pair<std::uint32_t, lib::vector<pipeline::image>>> array_element_and_images_pairs;
		for (auto it = changes.begin(); it != changes.end();) {
			std::uint32_t array_element = it->first;
			lib::vector<pipeline::image> images = { it->second };

			it = std::next(it);
			while (it != changes.end() && it->first == array_element + images.size()) {
				images.push_back(it->second);
				it = std::next(it);
			}

			array_element_and_images_pairs.push_back(std::make_pair(array_element,
																	std::move(images)));
		}

		changes.clear();

		return gl::bind(array_element_and_images_pairs);
	}

	const auto& operator[](std::size_t idx) const { return v[idx]; }

	auto size() const { return v.size(); }
	auto& get() const { return v; }
};

template <image_type type, int dimensions = image_dimensions_v<type>>
using storage_image_vector = image_vector<type, dimensions, gl::image_layout::general>;

}
}
