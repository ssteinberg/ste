// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <ostream>
#include <memory>

#include "log_streambuf.hpp"
#include "log_sink.hpp"

namespace StE {

class log_ostream : public std::ostream {
private:
	using streambuf_type = log_streambuf<512>;
	using sink_type = log_sink;

private:
	streambuf_type lsb;

public:
	log_ostream(std::unique_ptr<sink_type> &&sink, bool force_flush) : std::ostream(&lsb), lsb(std::move(sink), force_flush) {}
	~log_ostream() noexcept { flush(); }
};

}
