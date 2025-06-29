#include <log.h>
#include <ngrid.h>

int main() {
	Log::set_level(LEVEL_WARNING);
	NGrid::set_worgroup_size_x(256);
	NGrid A(5);
	A.fill_random_uniform_int(0, 9);
	A.print("before:", "|", 0, 0);
	A = A.transpose({ 1,0 });
	A.print("result:", "|", 0, 0);
}
