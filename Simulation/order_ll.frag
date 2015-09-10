
#version 440

struct node {
	vec3 p,n;
	uint ptr;
};

layout (binding = 0, r32ui)		uniform uimage2D heads;
layout (binding = 0, std430)	buffer ll { node nodes[]; };

node frags[25];

void main() {
	uint i = imageLoad(heads, ivec2(gl_FragCoord.xy)).r;

	int count = 0;
	while (i != 0xffffffff && count<25) {
		frags[count] = nodes[i];
		i = nodes[i].ptr;
		++count;
		//uint j = i;
		//uint next = nodes[j].ptr;
		//while (next != 0xffffffff && nodes[j].p.z < nodes[next].p.z) {
		//	vec3 tp = nodes[j].p;
		//	vec3 tn = nodes[j].n;
				
		//	nodes[j].p = nodes[next].p;
		//	nodes[j].n = nodes[next].n;
				
		//	nodes[next].p = tp;
		//	nodes[next].n = tn;
			
		//	j = next;
		//	next = nodes[next].ptr;
		//}

		//i = nodes[i].ptr;
	}

	for (int j=0; j<count; ++j) {
		node insert = frags[j];
		uint k = j;
		while (k>0 && insert.p.z > frags[k-1].p.z) {
			frags[k] = frags[k-1];
			--k;
		}
		frags[k] = insert;
	}

	gl_FragColor = vec4(frags[0].n,1);
}

