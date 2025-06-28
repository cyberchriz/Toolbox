#include <ngrid.h>

int main() {
	//Log::set_level(LEVEL_DEBUG);
	NGrid::set_worgroup_size_x(256);
	NGrid A(10, 10);
	A.fill_random_binary();
	A.print("before:");
	NGrid kernel(3, 3);
	kernel.fill_random_binary();
	kernel.print("kernel:");
	A = A.convolution(kernel, 1);
	A.print("convolution result:");
	A = A.pool_max({ 2,2 });
	A.print("pooled result:");
}
