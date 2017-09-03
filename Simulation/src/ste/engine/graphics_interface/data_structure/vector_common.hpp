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

private:
	void operator()(const command_buffer&, command_recorder& recorder) const override final {
		// Copy data
		for (auto& cmd : update_commands)
			recorder << cmd;
	}
};

// Resize command
template <class vector>
class vector_cmd_resize : public command {
	std::uint64_t old_size;
	std::uint64_t new_size;
	vector* v;

public:
	vector_cmd_resize(std::uint64_t old_size,
	                  std::uint64_t new_size,
	                  vector* v)
		: old_size(old_size),
		  new_size(new_size),
		  v(v) {}

	virtual ~vector_cmd_resize() noexcept {}

private:
	void operator()(const command_buffer&, command_recorder& recorder) const override final {
		// (Un)bind sparse (if needed) and update vector size
		if (new_size > old_size) {
			const range<std::uint64_t> bind = { old_size, new_size - old_size };
			recorder << v->get().cmd_bind_sparse_memory({}, { bind }, {}, {});
		}
		else if (new_size < old_size) {
			const range<std::uint64_t> unbind = { new_size, old_size - new_size };
			recorder << v->get().cmd_bind_sparse_memory({ unbind }, {}, {}, {});
		}
	}
};

// Insert command
template <class vector>
class vector_cmd_insert : public command {
	vector_cmd_update_buffer<vector> overwrite_cmd;
	std::uint64_t data_size;
	std::uint64_t location;
	vector* v;

public:
	vector_cmd_insert(const lib::vector<typename vector::value_type>& data_copy,
	                  std::uint64_t location,
	                  vector* v)
		: overwrite_cmd(data_copy,
		                location,
		                v),
		  data_size(static_cast<std::uint64_t>(data_copy.size())),
		  location(location),
		  v(v) {}

	virtual ~vector_cmd_insert() noexcept {}

private:
	void operator()(const command_buffer&, command_recorder& recorder) const override final {
		// Bind sparse (if needed) and update vector size
		const range<std::uint64_t> bind = { location, data_size };
		recorder << v->get().cmd_bind_sparse_memory({}, { bind }, {}, {});

		// Copy data
		recorder << overwrite_cmd;
	}
};

// Update (like insert but never bind memory) command
template <class vector>
class vector_cmd_update : public command {
	vector_cmd_update_buffer<vector> overwrite_cmd;
	std::uint64_t location;
	vector* v;

public:
	vector_cmd_update(const lib::vector<typename vector::value_type>& data_copy,
	                  std::uint64_t location,
	                  vector* v)
		: overwrite_cmd(data_copy,
		                location,
		                v),
		  location(location),
		  v(v) {}

	virtual ~vector_cmd_update() noexcept {}

private:
	void operator()(const command_buffer&, command_recorder& recorder) const override final {
		// Copy data
		recorder << overwrite_cmd;
	}
};

// Unbind sprase memory command
template <class vector>
class vector_cmd_unbind : public command {
	vector* v;
	std::uint64_t location;
	std::uint64_t count;

public:
	vector_cmd_unbind(std::uint64_t location,
	                  std::uint64_t count,
	                  vector* v)
		: v(v),
		  location(location),
		  count(count) {}

	virtual ~vector_cmd_unbind() noexcept {}

private:
	void operator()(const command_buffer&, command_recorder& recorder) const override final {
		// Unbind sparse (if possible) and update vector size
		const range<std::uint64_t> unbind = { location, count };
		recorder << v->get().cmd_bind_sparse_memory({ unbind }, {}, {}, {});
	}
};

}

}
}
