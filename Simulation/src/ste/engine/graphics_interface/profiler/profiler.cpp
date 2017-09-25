
#include <stdafx.hpp>
#include <profiler.hpp>

using namespace ste;
using namespace ste::gl::profiler;

profiler::profiler(const ste_context &ctx,
				   std::uint32_t max_atoms)
	: ctx(ctx),
	  timestamp_resolution_ns(ctx.device()->get_physical_device_descriptor().properties.limits.timestampPeriod),
	  max_atoms(max_atoms) {
	segments.emplace_back(0,
						  create_query_pool());
}

void profiler::end_segment() {
	const auto &segment = segments[current_segment];

	// If current segment was unused, nothing to end
	if (!segment.atoms.size())
		return;

	// Try to read segment results back from next query
	const auto next_segment_idx = (current_segment + 1) % segments.size();
	auto &next_segment = segments[next_segment_idx];
	assert(next_segment.atoms.size());

	// Attempt to read last query results from 
	struct timestamp_query_with_indicator {
		std::uint32_t timestamp, indicator;
	};
	timestamp_query_with_indicator last_query_results;
	if (!next_segment.query_pool.read_results<timestamp_query_with_indicator>(&last_query_results,
																			  static_cast<std::uint32_t>(next_segment.atoms.size() * 2) - 1,
																			  1,
																			  sizeof(timestamp_query_with_indicator),
																			  VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) ||
		last_query_results.indicator == 0) {
		// If results unavailable, create new segment and set it as current
		segments.emplace_back(segment.segment_idx,
							  create_query_pool());
		current_segment = segments.size() - 1;
		return;
	}

	// Otherwise, read timestamps
	lib::vector<timestamp_query_results_t> timestamps;
	timestamps.resize(next_segment.atoms.size() * 2);
	if (!next_segment.query_pool.read_results<timestamp_query_results_t>(timestamps.data(),
																		 0,
																		 static_cast<std::uint32_t>(next_segment.atoms.size() * 2),
																		 sizeof(timestamp_query_results_t))) {
		// Previous read was successful and indicator was positive, but this one fails???
		assert(false);

		segments.emplace_back(segment.segment_idx,
							  create_query_pool());
		current_segment = segments.size() - 1;
		return;
	}
	// Create results and notify
	segment_results_t results;
	results.segment_idx = next_segment.segment_idx;
	results.data.reserve(next_segment.atoms.size());
	for (std::size_t i = 0; i < next_segment.atoms.size(); ++i) {
		segment_result_atom a;
		a.name = next_segment.atoms[i];
		a.time_start_ms = timestamp_to_result_time(timestamps[i * 2]);
		a.time_end_ms = timestamp_to_result_time(timestamps[i * 2 + 1]);
		results.data.push_back(a);
	}

	segment_results_available_signal.emit(results);

	// Clear and use for next segment
	next_segment.atoms = atoms_t{};
	current_segment = next_segment_idx;
}
