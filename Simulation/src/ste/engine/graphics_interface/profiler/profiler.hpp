//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>

#include <vk_query_pool.hpp>
#include <command_recorder.hpp>
#include <cmd_write_timestamp.hpp>
#include <cmd_reset_query_pool.hpp>

#include <lib/vector.hpp>
#include <lib/string.hpp>
#include <alias.hpp>
#include <signal.hpp>

namespace ste {
namespace gl {
namespace profiler {

class profiler_atom {
	friend class profiler;

private:
	std::reference_wrapper<gl::command_recorder> recorder;
	gl::vk::vk_query<> query_end;
	gl::pipeline_stage stages_end;

public:
	profiler_atom(gl::vk::vk_query<> &&query_start,
				  gl::vk::vk_query<> &&query_end,
				  gl::command_recorder &recorder,
				  gl::pipeline_stage stages_start,
				  gl::pipeline_stage stages_end)
		: recorder(recorder),
		query_end(std::move(query_end)),
		stages_end(stages_end)
	{
		// Record start timestamp
		recorder << gl::cmd_write_timestamp(query_start, stages_start);
	}
	~profiler_atom() {
		// Record end timestamp
		recorder.get() << gl::cmd_write_timestamp(query_end, stages_end);
	}

	profiler_atom(profiler_atom&&) = default;
	profiler_atom &operator=(profiler_atom&&) = default;
};

class profiler {
private:
	using atoms_t = lib::vector<lib::string>;
	using timestamp_query_results_t = std::uint32_t;

	struct profiler_segment {
		std::uint64_t segment_idx;

		gl::vk::vk_timestamp_query_pool<> query_pool;
		atoms_t atoms;

		profiler_segment(std::uint64_t segment_idx,
						 gl::vk::vk_timestamp_query_pool<> &&p) : segment_idx(segment_idx), query_pool(std::move(p)) {}
	};

	struct segment_result_atom {
		float time_start_ms, time_end_ms;
		lib::string name;
	};

public:
	struct segment_results_t {
		std::uint64_t segment_idx;
		lib::vector<segment_result_atom> data;
	};
	using segment_results_available_signal_t = signal<segment_results_t>;

private:
	alias<const ste_context> ctx;
	float timestamp_resolution_ns;

	std::uint32_t max_atoms;

	lib::vector<profiler_segment> segments;
	std::uint64_t current_segment{ 0 };

	mutable segment_results_available_signal_t segment_results_available_signal;

private:
	auto create_query_pool() const {
		return gl::vk::vk_timestamp_query_pool<>(ctx.get().device(),
												 max_atoms * 2);
	}

	auto timestamp_to_result_time(timestamp_query_results_t timestamp) {
		return static_cast<float>(timestamp) * 1e-6f * timestamp_resolution_ns;
	}

public:
	/*
	*	@brief	Creates a new profiler object
	*
	*	@param	ctx			Context
	*	@param	max_atoms	Max atoms per segment
	*/
	profiler(const ste_context &ctx,
			 std::uint32_t max_atoms);
	~profiler() noexcept {}

	/*
	 *	@brief	Begins an atom. Atom ends when returned object goes out of scope.
	 *	
	 *	@param	recorder	Command recorder on which the atom will record
	 *	@param	name		Atom name
	 *	@param	stages		Earliest stage at which to record the END timestamp
	 */
	auto start_atom(gl::command_recorder &recorder,
					lib::string name,
					gl::pipeline_stage stages = gl::pipeline_stage::bottom_of_pipe) {
		auto &segment = segments[current_segment];
		const auto current_query_idx = static_cast<std::uint32_t>(segment.atoms.size() * 2);

		// Store atom name
		segment.atoms.emplace_back(std::move(name));

		// Reset query pool, if first atom
		if (current_query_idx == 0)
			recorder << gl::cmd_reset_query_pool(segment.query_pool, 0, max_atoms * 2);

		// Create atom
		return profiler_atom(segment.query_pool[current_query_idx],
							 segment.query_pool[current_query_idx + 1],
							 recorder,
							 gl::pipeline_stage::bottom_of_pipe,
							 stages);
	}

	/**
	 *	@brief	Ends the profiler segment. Must be called outside a renderpass.
	 *
	 *			Results are not immediately available.
	 *			end_segment() will try to read the results of previously completed but yet pending segments, if any results are available the results are
	 *			distributed via a signal. See get_segment_results_available_signal().
	 */
	void end_segment();

	/*
	 *	@brief	Returns a signal that is emitted when the profiler has new results
	 */
	auto& get_segment_results_available_signal() const { return segment_results_available_signal; }
};

}
}
}
