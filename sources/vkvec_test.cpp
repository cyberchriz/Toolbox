#include "../headers/vkvec.h"

int main() {
	vec A(10,10);
	A.fill_random_uniform_int(0, 9);
	A.print("A:");
	A.inverse().print("inverse:");
	(A * A.inverse()).print("A * A.inverse():");
} 
