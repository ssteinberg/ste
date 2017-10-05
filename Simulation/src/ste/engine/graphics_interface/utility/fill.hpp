//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>
#include <ste_context.hpp>
#include <job.hpp>

#include <host_write_buffer.hpp>

#include <array.hpp>
#include <vector.hpp>
#include <stable_vector.hpp>

namespace ste {
namespace gl {

template <typename T>
auto fill(array<T> &dst,
		  lib::vector<T> &&data,
		  std::size_t offset = 0) {
	const auto &ctx = dst->parent_context();
	auto future = ctx.engine().task_scheduler().schedule_now([&dst, data = std::move(data), offset]() {
		_internal::host_write_buffer(dst->parent_context(),
									 dst,
									 data.data(),
									 data.size(),
									 offset);
	});

	return make_job(std::move(future));
}

template <typename T, std::uint64_t max_sparse_size_bytes>
auto fill(stable_vector<T, max_sparse_size_bytes> &dst,
		  lib::vector<T> &&data,
		  std::size_t offset = 0) {
	const auto &ctx = dst->parent_context();
	auto future = ctx.engine().task_scheduler().schedule_now([&dst, data = std::move(data), offset]() {
		_internal::host_write_buffer_and_resize(dst->parent_context(),
												dst,
												data.data(),
												data.size(),
												offset);
	});

	return make_job(std::move(future));
}

template <typename T, std::uint64_t max_sparse_size_bytes>
auto fill(vector<T, max_sparse_size_bytes> &dst,
		  lib::vector<T> &&data,
		  std::size_t offset = 0) {
	const auto &ctx = dst->parent_context();
	auto future = ctx.engine().task_scheduler().schedule_now([&dst, data = std::move(data), offset]() {
		_internal::host_write_buffer_and_resize(dst->parent_context(),
												dst,
												data.data(),
												data.size(),
												offset);
	});

	return make_job(std::move(future));
}

}
}
