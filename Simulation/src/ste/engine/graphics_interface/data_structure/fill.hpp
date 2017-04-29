//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <job.hpp>

#include <copy_data_buffer.hpp>

#include <array.hpp>
#include <stable_vector.hpp>

namespace ste {
namespace gl {

template <typename T>
auto fill(array<T> &dst,
		  std::vector<T> &&data,
		  std::size_t offset = 0) {
	const auto &ctx = dst->parent_context();
	auto future = ctx.engine().task_scheduler().schedule_now([&dst, data = std::move(data), offset]() {
		_internal::copy_data_buffer(dst->parent_context(),
									dst.get(),
									data,
									offset);
	});

	return make_job(std::move(future));
}

template <typename T, std::uint64_t minimal_atom_size, std::uint64_t max_sparse_size>
auto fill(stable_vector<T, minimal_atom_size, max_sparse_size> &dst,
		  std::vector<T> &&data,
		  std::size_t offset = 0) {
	const auto &ctx = dst->parent_context();
	auto future = ctx.engine().task_scheduler().schedule_now([&dst, data = std::move(data), offset]() {
		_internal::copy_data_buffer(dst->parent_context(),
									dst.get(),
									data,
									offset);
	});

	return make_job(std::move(future));
}

}
}
