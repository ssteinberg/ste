// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

namespace StE {
namespace LLR {

class gpu_sync {
	GLsync sync;

public:
	gpu_sync() : sync(nullptr) {}
	~gpu_sync() { destroy(); }

	gpu_sync(gpu_sync &&bl) : sync(bl.sync) { bl.sync = nullptr; }
	gpu_sync &operator=(gpu_sync &&bl) {
		destroy();
		sync = bl.sync;
		bl.sync = nullptr;
		return *this;
	}
	gpu_sync(const gpu_sync &bl) = delete;
	gpu_sync &operator=(const gpu_sync &bl) = delete;

	void server_wait() const {
		glFlush();
		glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
	}

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
