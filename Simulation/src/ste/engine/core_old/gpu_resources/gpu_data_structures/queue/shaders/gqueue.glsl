
struct queue_node {
	int valid;
	queue_type data;
};

layout(std430, binding = queue_binding_index) coherent buffer queue_buffer {
	queue_node queue_nodes[];
};
layout(std430, binding = queue_binding_index + 1) coherent buffer queue_tail_buffer {
	uint queue_tail;
};
layout(binding = queue_binding_index + 2) uniform atomic_uint queue_head;

void queue_push(queue_type data) {
	queue_node n;
	n.data = data;
	n.valid = 1;

	uint size = queue_nodes.length();

	uint idx = atomicCounterIncrement(queue_head) % size;
	queue_nodes[idx] = n;
}

bool queue_pop(out queue_type data) {
	memoryBarrierBuffer();

	uint size = queue_nodes.length();

	uint idx;
	queue_node n;
	do {
		idx = queue_tail;
		n = queue_nodes[idx];
		if (n.valid == 0)
			return false;
	} while(atomicCompSwap(queue_tail, idx, (idx + 1) % size) != idx);
	queue_nodes[idx].valid = 0;

	data = n.data;

	return true;
}
