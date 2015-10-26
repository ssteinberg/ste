// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class buffer_lock {
	GLsync sync;

public:
	buffer_lock() : sync(nullptr) {}
	~buffer_lock() { destroy(); }

	buffer_lock(buffer_lock &&bl) : sync(bl.sync) { bl.sync = nullptr; }
	buffer_lock &operator=(buffer_lock &&bl) {
		destroy();
		sync = bl.sync;
		bl.sync = nullptr;
		return *this;
	}
	buffer_lock(const buffer_lock &bl) = delete;
	buffer_lock &operator=(const buffer_lock &bl) = delete;

	void wait() const {
		glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}
	void lock() {
		destroy();
		sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}

	void destroy() {
		if (sync)
			glDeleteSync(sync);
	}
};

}
}
