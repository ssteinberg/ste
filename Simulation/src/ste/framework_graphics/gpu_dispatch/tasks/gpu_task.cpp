
#include "stdafx.hpp"
#include "gpu_task.hpp"

#include "gpu_task_dispatch_queue.hpp"

using namespace StE::Graphics;

void gpu_task::set_modified() const {
	if (parent_queue)
		parent_queue->signal_task_modified(this);
}
