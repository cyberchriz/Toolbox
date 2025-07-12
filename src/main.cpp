#include <log.h>
#include <ngrid.h>

int main() {
	Log::set_level(LEVEL_WARNING);
	NGrid::set_workgroup_size_1d(256);
	NGrid A(10, 20);
	A.fill_random_uniform_int(-9, 9);
	A.print("\nsource matrix A:");

	NGrid L, U, P;
	A.lu_decomp(L, U, P);
	L.print("\nmatrix L:");
	U.print("\nmatrix U:");
	P.print("\nmatrix P:");
	(P.transpose() * L * U).round().print("\nA = P.transpose * L * U:");

	// check for square matrix A
	if (A.get_shape()[0] == A.get_shape()[1]) {
		NGrid L_inv = L.l_inverse();
		NGrid U_inv = U.u_inverse();

		L_inv.print("\nL_inverse:", "|", 0, 1, 16);
		U_inv.print("\nU_inverse:", "|", 0, 1, 16);

		(L * L_inv).print("\ntest calculation: L * L_inv (should result in identity matrix if L_inv has been calculated correctly) ");
		(U * U_inv).print("\ntest calculation: U * U_inv (should result in identity matrix if U_inv has been calculated correctly) ");

		NGrid A_inv;
		A_inv = U_inv * L_inv * P;
		A_inv.print("\nA_inv = U_inv * L_inv * P =");

		(A * A_inv).print("\ntest calculation: A * A_inv (should result in identity matrix if A_inv has been calculated correctly) ");
	}

	NGrid A_inv = A.inverse();
	A_inv.print("\nA.inverse(100):");
	(A * A_inv).print("\ntest of A * A.inverse():");
}
