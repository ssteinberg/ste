
#type compute
#version 450

const int queue_binding_index = 1;
struct queue_type {
	uint i;
};

#include "gqueue.glsl"

layout(std430, binding = 0) coherent buffer obuf {
	queue_type o[];
};

layout(local_size_x = 128) in;

shared int threads_waiting;

void main() {
	threads_waiting = 0;
	o[700].i = 1;
	barrier();

	int id = int(gl_GlobalInvocationID.x);
	if (id<64) {
		for (int i=0;i<10;++i) {
			queue_type v;
			v.i = uint(i + id * 10 + 1);
			queue_push(v);
		}
	}
	else {
		for (int i=0;i<3;++i) {
			queue_type v;
			v.i = uint(0);
			queue_push(v);
		}

		while (true) {
			queue_type v;

			atomicAdd(threads_waiting, 1);
			while (!queue_pop(v)) {
				if (threads_waiting == 64) return;
			}
			atomicAdd(threads_waiting, -1);

			if (v.i > 800)
				o[700].i = 666;
			o[v.i].i = v.i;
		}
	}
	
	barrier();
}
