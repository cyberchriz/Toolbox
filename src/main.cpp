#include <log.h>
#include <ngrid.h>

int main() {
	Log::set_level(LEVEL_WARNING);
	NGrid::set_workgroup_size_1d(256);
	NGrid A(100);
	A.fill_random_uniform_int(0, 9);
	A.print("\nmatrix A:");

	NGrid B = A;
	B.print("\nmatrix B (copied from A):");

	B *= 2; // multiply B by 2
	B.print("\nmatrix B after multiplying by 2 (as beta1):");

	NGrid C(100);
	C.fill_random_uniform_int(-2, 2);

	C.print("\nlet's add some randomness: matrix C (random values -2 to 2):");

	B += C; // add C to B
	B.print("\nmatrix B after adding C:");

	B += 5; // add 5 to B
	B.print("\nmatrix B after adding 5 (as y_intercept):");

	Log::force("Now let's check if the reggression works; we will use A as x and B as y");

	auto result = A.regression(B);
	result.print();

}
