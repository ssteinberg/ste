// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"

namespace StE {
namespace Core {

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

	void wait() const {
		glFlush();
		glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
	}

	void client_wait() const {
		glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
	}

	bool check() const {
		if (!sync)
			return true;
		auto result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
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
