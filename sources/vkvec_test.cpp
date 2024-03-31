#include "../headers/vkvec.h"

int main() {
	vec A(1,20);
	A.fill_random_uniform_int(0,9);
	A.print("source");
	(A > 5).print("A>5:");
} 
