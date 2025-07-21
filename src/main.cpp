#include <log.h>
#include <ngrid.h>

int main() {
	// +=================================+   
	// | Functionality Demonstration     |
	// +=================================+
	Log::force("\nThis is a test output for the NGrid library, demonstrating its functionality.");
	Log::force("This library implements a vast collection of parallel compute operations on the GPU, using Vulkan.");

	Log::force("\n'NGrid' in this case is used as a typename for n-dimensional data-structures.");
	Log::force("\nAll values are by default stored as 32bit floating point");
	Log::force("\nPlease note that there is no theoretical limit to the max number of dimensions for base cases,");
	Log::force("but some of the used compute shaders have a hard-coded limit to handle max 10 dimensions.");

	Log::force("\nMost of the methods (except for setter or fill functions) are NON-DESTRUCTIVE by design. In other words, they typically");
	Log::force("leave the source object unmodified and return a result NGrid object for the given operation.");
	Log::force("The assignment operator can be used whenever this result is supposed to be written to the source array of the operation.");
	Log::force("Examples:");
	Log::force("\nA = A.transpose()");
	Log::force("A = A.pow(2);");
	Log::force("A = A * 2");
	Log::force("A += 0.5            ...etc");
	Log::force("\nThe outputs in the following examples are made by using the NGrid::print() method.");

	Log::force("\n+=================================+\n| Constructors & Destructors      |\n+=================================+");
	Log::force("\nLet's start with a 10x10 2d matrix using the variadic template constructor: template<typename... Args> NGrid(Args... args);");
	Log::force("NGrid A(10,10);\nA.fill(5); // initializing all elements with a value of 5.0f");
	NGrid A(10, 10);
	A.fill(5);
	A.print("\nresult (A):");

	Log::force("\nNow let's use a constructor with a std::vector instead for the shape: NGrid(const std::vector<uint32_t>& shape);");
	Log::force("std::vector<uint32_t> shape = {3,5);\nNGrid B(shape); \nB.fill_zero();");
	std::vector<uint32_t> shape = { 3,5 };
	NGrid B(shape);
	B.fill_zero();
	B.print("\nresult (B):");

	Log::force("\nA std::initializer_list<uint32_t> can also be used for the shape:");
	Log::force("NGrid C({4,4});\nLet's fill this matrix with random numbers between 0-9:\nC.fill_random_int(0,9);");
	NGrid C({ 4,4 });
	C.fill_random_int(0, 9);
	C.print("\nresult (C):");

	Log::force("\nAn NGrid array can also be constructed by directly copying the contents of an existing 1-dimensional std::vector<float_t>:");
	Log::force("std::vector<float_t> example = { 1,2,3,4,5 };\nNGrid D(example);");
	std::vector<float_t> example = { 1,2,3,4,5 };
	NGrid D(example);
	D.print("\nresult (D):");

	Log::force("\n+=================================+\n| Assignment                      |\n+=================================+");
	Log::force("\nHere's an example of copy assignment, using 'operator=', in this example let's reassign D to C (see above):");
	Log::force("D = C;");
	D = C;
	D.print("\nupdated output for D:");
	Log::force("As you can see, the reassignment operation not only copied the values from 'C', but also its dimensions and shape.");

	Log::force("\nWe can also use the assignment operator to assign the contents of a std::vector<float_t>, i.e. as an alias for set(const std::vector<float_t>& data)");
	Log::force("Let's use the std::vector<float_t> called 'example' from above and reassign D again:");
	Log::force("D = example;");
	D = example;
	D.print("\nupdated result (D):");

	Log::force("\nWe can also assign a std::array. For this to work, the source array must not have less elements than the target NGrid.");
	Log::force("Excess elements in the source array are simply ignored, whilst a lower number of elements result in undefined behavior or access violation!");
	Log::force("float_t array[5] = {6, 7, 8, 9, 10};\nD = array;");
	float_t array[5] = { 6, 7, 8, 9, 10 };
	D = array;
	D.print("\nupdated result (D):");

	Log::force("\n+=================================+\n| Getters & Setters               |\n+=================================+");
	Log::force("\nA std::initializer_list<uint32_t> argument can be used to update an individual NGrid element via its multidimensional index:");
	Log::force("NGrid E(5,5);\nE.fill_zero();\E.set({2,2},1)");
	NGrid E(5, 5);
	E.fill_zero();
	E.set({ 2,2 }, 1);
	E.print("\nresult (E):");

	Log::force("\nAlternatively, std::vector<uint32_t> also works for the index argument");
	Log::force("std::vector<uint32_t> id = {0,1};\nE.set(id, 3);");
	std::vector<uint32_t> id = { 0,1 };
	E.set(id, 3);
	E.print("\nupdated result (E):");

	Log::force("\nWe've seen earlier that the assignment operator can be used to copy the contents of a std::vector<float_t> to a 1d NGrid array.");
	Log::force("By using the method void set(const std::vector<float_t>& data, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements)");
	Log::force("there's also the option to pass offset arguments and/or only make a partial copy of the source vector.");
	Log::force("As an example, let's only copy 3 elements of the source, at a source offset of 2, with a target offset of 4");
	Log::force("std::vector<float_t> source = { 0,1,2,3,4,5,6,7,8,9 };\nNGrid F(10);\nF.fill_zero();\nF.set(source, 3, 2, 4);");
	std::vector<float_t> source = { 0,1,2,3,4,5,6,7,8,9 };
	NGrid F(10);
	F.fill_zero();
	F.set(source, 3, 2, 4);
	F.print("\nresult (F) after operation:");

	Log::force("\nAccordingly, this type of assignment with offset arguments also works for copying from a std::array<float_t> by using the related method");
	Log::force("void set(const float_t* data, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements)");

	Log::force("\nThis type of assignment with offset arguments also works for copying from a second NGrid by using the method");
	Log::force("void NGrid::set(const NGrid& other, uint32_t copied_elements, uint32_t source_offset_elements, uint32_t target_offset_elements)");
	Log::force("This method copies based on flat indexing and is therefore intended to be used primarily on 1d NGrid objects");

	Log::force("\If a multidimensional source is used, a multi-dimensional offset index is required as follows:");
	Log::force("As an example, let's take matrix A from above (a 10x10 matrix filled with fives) and insert the random 4x4 matrix C (see above) at offset index {2,3}:");
	A.set(C, { 2,3 });
	A.print("Result (A) after inserting (C) at offset {2,3}");

	Log::force("\nNotes:\n  (1) The offset argument can be either type std:initializer_list<uint32_t> or std::vector<uint32_t>");
	Log::force("  (2) The target matrix doesn't get reshaped or resized. Excess elements beyond the target's boundaries are ignored.");
	Log::force("  (3) In case only a partial region of the 'other' NGrid is supposed to be inserted, this partial copy can be extracted");
	Log::force("      by applying the NGrid::subgrid() method to 'other' first");

	Log::force("\nLet's now move on to getter functions.");
	Log::force("First let's create a vector of random floats:\nNGrid floats(10);\nfloats.fill_random(0,1); // fill with random floats from a uniform distribution within range 0-1");
	NGrid floats(10);
	floats.fill_random(0, 1);
	floats.print("\nresult (floats):");
	Log::force("Now let's retrieve the second element via its flat index (indexing starts from 0):\nfloat_t x = floats.get(1);");
	float_t x = floats.get(1);
	Log::force("result: x = ", x);

	Log::force("\nWe can also copy the entire NGrid to a std::vector<float_t>:");
	Log::force("std::vector<float_t> as_vector = floats.get();\nx = as_vector[1];");
	std::vector<float_t> as_vector = floats.get();
	x = as_vector[1];
	Log::force("result: x = ", x, " (should be the same result as above)");

	Log::force("\nCopying to a std::vector<float_t> can also be done partially and/or with an offset argument:");
	Log::force("as_vector = floats.get(5, 1); // copy 5 elements of 'floats' starting at an offset of 1");
	as_vector = floats.get(5, 1);
	Log::force("Now element [0] in the result vector should be the same as element [1] of the source (due to the offset of 1)");
	x = as_vector[0];
	Log::force("x = as_vector[0];\nresult: x = ", x, " (again, if everything works as intendec, this value should be the same as above)");

	Log::force("\nHere are examples of some more getter functions:");
	Log::force("  Buffer<float_t>* NGrid::get_buffer(): returns a pointer to the underlying raw data buffer object");
	Log::force("  Buffer<uint32_t>* NGrid::get_shape_buffer(): returns a pointer to the raw shape buffer object");
	uint32_t n = A.get_dimensions();
	Log::force("  uint32_t n = A.get_dimensions(); // result: n = ", n);
	uint32_t y = A.get_size(1);
	Log::force("  uint32_t y = A.get_size(1); // return the size of the second dimension (indexing starts from 0) of the 10x10 matrix A; result: y = ", y);
	Log::force("  A_elements = A.get_elements(); // result: A_elements = ", A.get_elements());
	Log::force("  std::vector<uint32_t> shape = A.get_shape(); // result: shape[0] = ", A.get_shape()[0], ", shape[1] = ", A.get_shape()[1]);
	Log::force("  std::string shape_str = A.get_shapestring(); // result: shape_str = ", A.get_shapestring());

	Log::force("\nThe 'subgrid' method has already been mentioned, here's how it works:");
	Log::force("\nLet's create a random 5x5 matrix:\n NGrid G(5,5);\nG.fill_random(10,99); // fill with random values within range 10-99");
	NGrid G(5, 5);
	G.fill_random_int(10, 99);
	G.print("\nG = ");
	Log::force("\nNow let's extract a 3x3 subgrid at an offset of {1,1} relative to the origin of the source; then reassign the result to the original matrix G:");
	Log::force("G = G.subgrid({ 1,1 }, { 3,3 });");
	G = G.subgrid({ 1,1 }, { 3,3 });
	G.print("\nresult (G) after the operation:");
	Log::force("Note: the offset and shape arguments can either be both type std::initializer_list<uint32_t> or both type std::vector<uint32_t>");

	Log::force("\n+=================================+\n| Fill / Initialize               |\n+=================================+");
	Log::force("\nWe've already seen examples of the methods fill(float_t value), fill_zero(), fill_random() and fill_random_int().");
	Log::force("fill_random() and fill_random_int() generate random values from a uniform distribution.");
	Log::force("The function names fill_random_uniform() and fill_random_uniform_int() are aliases which can be used interchangeably.");
	Log::force("On the other hand, the method void fill_random_gaussian(const float_t mu = 0.0f, const float_t sigma = 1.0f)");
	Log::force("initializes with values from a Gaussian normal distribution (with the specified mean and standard deviation).");

	Log::force("\nThe method fill_random_binary(float_t ratio) fills with zeros and ones at the given ratio");
	Log::force("For example, in order to initialize randromly with 20% zeros / 80% ones, we could do the following:");
	Log::force("NGrid H(10,20);\nH.fill_random_binary(0.2);");
	NGrid H(20, 20);
	H.fill_random_binary(0.2);
	H.print("\nresult (H):");

	Log::force("\nWe could also fill/initialize with a random sign (-1 / 1):\nH.fill_random_sign(0.5); // ratio 0.5 is also by default if no ratio is explicitly given");
	H.fill_random_sign(0.5);
	H.print("\nresult (H):");

	Log::force("\nWe can also fill/initialize with a continuous range (given a start and step argument); this applies to all dimensions, for example: ");
	Log::force("H.fill_range(99,-1);");
	H.fill_range(99, -1);
	H.print("\nresult (H):");

	Log::force("\nfill_dropout(float_t ratio) can be used to randomly set a certain percentage to zero (which can e.g. be useful for neural network applications), example:");
	Log::force("H.fill(7);\nH.fill_dropout(0.2);");
	H.fill(7);
	H.fill_dropout(0.2);
	H.print("\nresult (H):");

	Log::force("\nThe fill_index() method assigns the flat index to each element, example:\nH.fill_index();");
	H.fill_index();
	H.print("\nresult (H):");

	Log::force("\nThe fill_identity() method initializes with the 'identity matrix' (diagonal all ones, rest 0)");
	Log::force("H.fill_identity();");
	H.fill_identity();
	H.print("\nresult (H):");

	// ... to be continued
}
