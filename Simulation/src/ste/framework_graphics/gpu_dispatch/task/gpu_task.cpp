
#include "stdafx.h"
#include "gpu_task.h"

#include "gpu_task_dispatch_queue.h"

using namespace StE::Graphics;

void gpu_task::set_modified() const {
	if (parent_queue)
		parent_queue->signal_task_modified(shared_from_this());
}
