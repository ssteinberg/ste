//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_device_queue_batch.hpp>

#include <vk_buffer_sparse.hpp>

#include <device_sparse_memory_bind.hpp>

#include <lib/vector.hpp>
#include <lib/flat_map.hpp>

namespace ste {
namespace gl {

/*
*	@brief	A sparse binding device batch.
*/
template <typename UserData = void>
class device_sparse_binding_batch : public ste_device_queue_batch_user_data<UserData> {
	friend class ste_device_queue;
	using Base = ste_device_queue_batch_user_data<UserData>;

protected:
	using Base::ctor;
	using Base::fence_strong;

private:
	lib::flat_map<VkBuffer, lib::vector<VkSparseMemoryBind>> buffer_bind_map;
	// Sparse images currently unsupported
	//	lib::flat_map<VkImage, lib::vector<VkSparseImageMemoryBind>> image_bind_map;
	//	lib::flat_map<VkImage, lib::vector<VkSparseMemoryBind>> opaque_image_bind_map;

protected:
	void submit(const vk::vk_queue<> &q) const override final {
		// Create semaphore handles
		auto wait_semaphore_handles = lib::vector<VkSemaphore>(Base::wait_semaphores.begin(),
															   Base::wait_semaphores.end());
		auto signal_semaphore_handles = Base::vk_semaphores(Base::signal_semaphores);

		// Creates sparse binding commands
		lib::vector<VkSparseBufferMemoryBindInfo> buffer_binds;
		lib::vector<VkSparseImageMemoryBindInfo> image_binds;
		lib::vector<VkSparseImageOpaqueMemoryBindInfo> image_opaque_binds;

		buffer_binds.reserve(buffer_bind_map.size());
		for (auto &bp : buffer_bind_map) {
			VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
			buffer_memory_bind_info.buffer = bp.first;
			buffer_memory_bind_info.bindCount = static_cast<std::uint32_t>(bp.second.size());
			buffer_memory_bind_info.pBinds = bp.second.data();

			buffer_binds.push_back(buffer_memory_bind_info);
		}

		// Submit bind commands
		q.submit_bind_sparse(buffer_binds,
							 image_binds,
							 image_opaque_binds,
							 wait_semaphore_handles,
							 signal_semaphore_handles,
							 &(*fence_strong)->get_fence());

		// Signal fence's host-side future
		(*fence_strong)->signal();
	}

public:
	template <typename S = UserData>
	device_sparse_binding_batch(std::enable_if_t<std::is_void_v<S>, ctor>,
								std::uint32_t queue_index,
								typename Base::fence_ptr_strong_t &&fence)
		: Base(queue_index,
			   std::move(fence)) {}

	template <typename S = UserData, typename... UserDataArgs>
	device_sparse_binding_batch(std::enable_if_t<!std::is_void_v<S>, ctor>,
								std::uint32_t queue_index,
								typename Base::fence_ptr_strong_t &&fence,
								UserDataArgs &&... user_data_args)
		: device_sparse_binding_batch(ctor{},
									  queue_index,
									  std::move(fence),
									  std::forward<UserDataArgs>(user_data_args)...) {}

	virtual ~device_sparse_binding_batch() noexcept {}

	device_sparse_binding_batch(device_sparse_binding_batch &&) = default;
	device_sparse_binding_batch &operator=(device_sparse_binding_batch &&) = default;

	/*
	 *	@brief	(Un)binds sprase buffer memory
	 */
	void bind(const vk::vk_buffer_sparse<> &buffer,
			  const lib::vector<device_sparse_memory_bind> &binds) {
		lib::vector<VkSparseMemoryBind> vk_binds;

		for (auto &bind : binds) {
			VkSparseMemoryBind b = {};

			b.resourceOffset = static_cast<std::size_t>(bind.resource_offset_bytes);
			b.size = static_cast<std::size_t>(bind.size_bytes);

			if (bind.allocation != nullptr) {
				// Bind
				assert(*bind.allocation);
				b.memory = *bind.allocation->get_memory();
				b.memoryOffset = static_cast<std::size_t>((**bind.allocation).get_offset());
			}
			else {
				// Unbind
				b.memory = vk::vk_null_handle;
			}

			vk_binds.push_back(b);
		}

		// Append to map
		std::copy(vk_binds.begin(), 
				  vk_binds.end(), 
				  std::back_inserter(buffer_bind_map[buffer]));
	}
};

}
}
