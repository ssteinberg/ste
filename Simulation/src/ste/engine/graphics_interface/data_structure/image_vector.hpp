//	StE
// � Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <ste_resource_traits.hpp>

#include <image.hpp>
#include <texture.hpp>
#include <pipeline_resource_binder.hpp>

#include <anchored.hpp>
#include <alias.hpp>

#include <lib/range_list.hpp>
#include <lib/flat_map.hpp>
#include <lib/shared_ptr.hpp>

#include <optional.hpp>
#include <allow_type_decay.hpp>
#include <mutex>
#include <atomic>

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

	using image_t = pipeline::image;

	using changes_set_t = lib::flat_map<std::uint32_t, image_t>;

public:
	using texture_t = texture<type, dimensions>;

	class slot_t : anchored, public allow_type_decay<slot_t, texture_t> {
		friend class image_vector;
		struct token {};

	private:
		texture_t tex;

		alias<image_vector> storage;
		std::uint32_t slot_idx{ 0 };

	public:
		slot_t(token,
			   texture_t &&tex,
			   image_vector &parent,
			   std::uint32_t slot_idx)
			: tex(std::move(tex)),
			storage(parent),
			slot_idx(slot_idx)
		{}
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
		changes.insert(std::make_pair(idx, std::move(img)));
	}

	/**
	*	@brief	Marks an element with a tombstone. Tombstones can be consumed by calling insert(), overwriting the tombstone
	*			with new data.
	*/
	void erase(std::uint32_t idx) {
		assert(idx < size());

		// Annihilate slot
		{
			std::unique_lock<std::mutex> l(general_mutex);

			// Update changes
			changes.erase(idx);
		}

		{
			std::unique_lock<std::mutex> lt(tombstones_mutex);

			// Mark tombstone
			const tombstone_range t(idx, 1);
			tombstones.add(t);
		}
	}
	/**
	*	@brief	Marks an element with a tombstone. Tombstones can be consumed by calling allocate_slot(), overwriting the tombstone
	*			with new data.
	*/
	void erase(const slot_t &element) {
		erase(element.get_slot_idx());
	}

private:
	std::atomic<std::uint32_t> count{ 0 };
	tombstone_ranges_t tombstones;

	mutable changes_set_t changes;

	mutable std::mutex tombstones_mutex;
	mutable std::mutex general_mutex;

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
		// Find a location for the slot: If there are tombstones, replace one of them with new element, if possible
		optional<std::uint32_t> location;
		{
			std::unique_lock<std::mutex> l(tombstones_mutex);

			if (tombstones.size()) {
				location = std::prev(tombstones.end())->start;
				tombstones.pop_back();
			}
		}

		// If no tombstones, use a location past the vector's end.
		if (!location)
			location = count.fetch_add(1);

		// Create slot and pipeline image, we can do that without a lock
		value_type val = lib::allocate_shared<slot_t>(slot_t::token(),
													  std::move(tex),
													  *this,
													  location.get());
		auto img = image_t(val->tex, layout);
		
		// Update changes data
		{
			std::unique_lock<std::mutex> l(general_mutex);
			add_change(location.get(),
					   std::move(img));
		}

		return val;
	}

	/**
	 *	@brief	Returns a vector of pipeline_resource_binder instances that bind the minimal amount of textures that changed since last 
	 *			call to binder(). 
	 *			Same return type as gl::bind, designed to be passed to a pipeline binding point.
	 */
	auto binder() const {
		// Make a copy of the changes vector, safely, and clear the changes.
		changes_set_t changes_copy;
		{
			std::unique_lock<std::mutex> l(general_mutex);

			changes_copy = changes;
			changes.clear();
		}

		// Create binding data
		lib::vector<std::pair<std::uint32_t, lib::vector<pipeline::image>>> array_element_and_images_pairs;
		for (auto it = changes_copy.begin(); it != changes_copy.end();) {
			std::uint32_t array_element = it->first;
			lib::vector<pipeline::image> images = { it->second };

			it = std::next(it);
			while (it != changes_copy.end() && it->first == array_element + images.size()) {
				images.push_back(it->second);
				it = std::next(it);
			}

			array_element_and_images_pairs.push_back(std::make_pair(array_element,
																	std::move(images)));
		}

		return gl::bind(array_element_and_images_pairs);
	}

	auto size() const { return count.load(); }
};

template <image_type type, int dimensions = image_dimensions_v<type>>
using storage_image_vector = image_vector<type, dimensions, gl::image_layout::general>;

}
}
