//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <command_recorder.hpp>
#include <cmd_update_buffer.hpp>

#include <lib/blob.hpp>
#include <lib/vector.hpp>

namespace ste {
namespace gl {

namespace _internal {

template <class vector>
class vector_cmd_update_buffer : public command {
	lib::vector<cmd_update_buffer> update_commands;

public:
	vector_cmd_update_buffer(const lib::vector<typename vector::value_type>& data_copy,
	                         std::uint64_t location,
	                         vector* v) {
		// Calculate the maximal amount of elements possible to update in a single update buffer command
		auto elements_per_chunk = cmd_update_buffer::maximal_update_bytes / sizeof(typename vector::value_type);

		// Create update buffer commands
		for (std::size_t e = 0; e < data_copy.size(); e += elements_per_chunk, location += elements_per_chunk) {
			auto chunk_count = glm::min(elements_per_chunk, data_copy.size() - e);

			update_commands.emplace_back(buffer_view(v->get(),
			                                         location,
			                                         chunk_count),
			                             lib::blob(data_copy.data() + e,
			                                       chunk_count * sizeof(typename vector::value_type)));
		}
	}

	virtual ~vector_cmd_update_buffer() noexcept {}

	vector_cmd_update_buffer(vector_cmd_update_buffer&&) = default;
	vector_cmd_update_buffer &operator=(vector_cmd_update_buffer&&) = default;

private:
	void operator()(const command_buffer&, command_recorder& recorder) && override final {
		// Copy data
		for (auto&& cmd : update_commands)
			recorder << std::move(cmd);
	}
};

// Resize command
template <class vector>
class vector_cmd_resize : public command {
	alias<const ste_context> ctx;

	std::uint64_t old_size;
	std::uint64_t new_size;
	vector* v;

public:
	vector_cmd_resize(const ste_context &ctx,
					  std::uint64_t old_size,
	                  std::uint64_t new_size,
	                  vector* v)
		: ctx(ctx),
		  old_size(old_size),
		  new_size(new_size),
		  v(v) {}

	virtual ~vector_cmd_resize() noexcept {}

	vector_cmd_resize(vector_cmd_resize&&) = default;
	vector_cmd_resize &operator=(vector_cmd_resize&&) = default;

private:
	void operator()(const command_buffer&, command_recorder& recorder) && override final {
		// Command creates a sparse binding batch, submits and creates a dependency

		// (Un)bind sparse, if needed
		lib::vector<range<std::uint64_t>> unbind_regions;
		lib::vector<range<std::uint64_t>> bind_regions;
		if (new_size > old_size)
			bind_regions.emplace_back(old_size, new_size - old_size);
		else if (new_size < old_size)
			unbind_regions.emplace_back(new_size, old_size - new_size);
		else
			return;

		// Dependency semaphore
		auto sem = ctx.get().device().get_sync_primitives_pools().semaphores().claim();

		// Enqueue task
		lib::vector<const semaphore*> signal_semaphores;
		signal_semaphores.emplace_back(&sem.get());
		v->get().bind_sparse_memory(unbind_regions,
									bind_regions,
									{},
									std::move(signal_semaphores)).get();

		// Add dependency
		this->add_dependency(wait_semaphore(std::move(sem), 
											pipeline_stage::bottom_of_pipe));
	}
};

// Insert command
template <class vector>
class vector_cmd_insert : public command {
	vector_cmd_update_buffer<vector> overwrite_cmd;
	vector_cmd_resize<vector> resize_cmd;

public:
	vector_cmd_insert(const ste_context &ctx,
					  const lib::vector<typename vector::value_type>& data_copy,
	                  std::uint64_t location,
	                  vector* v)
		: overwrite_cmd(data_copy,
		                location,
		                v),
		resize_cmd(ctx,
				   location,
				   location + static_cast<std::uint64_t>(data_copy.size()),
				   v) {}

	virtual ~vector_cmd_insert() noexcept {}

	vector_cmd_insert(vector_cmd_insert&&) = default;
	vector_cmd_insert &operator=(vector_cmd_insert&&) = default;

private:
	void operator()(const command_buffer&, command_recorder& recorder) && override final {
		// Bind sparse (if needed) and update vector size
		recorder << std::move(resize_cmd);
		// Copy data
		recorder << std::move(overwrite_cmd);
	}
};

}

}
}
