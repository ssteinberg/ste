
#include <stdafx.hpp>
#include <vk_descriptor_set.hpp>
#include <vk_descriptor_pool.hpp>

using namespace StE::GL;

void vk_descriptor_set::free() {
	if (set && pool.allows_freeing_individual_sets()) {
		vkFreeDescriptorSets(device, pool, 1, &set.get());
		set = none;
	}
}
