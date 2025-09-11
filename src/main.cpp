

// THIS "MAIN" FILE IS ONLY FOR THE PURPOSES OF DEMONSTRATION, TESTING AND DEBUGGING OF THE ngrid.h LIBRARY

// NGrid is a class for parallel floating point data structure computations, including advanced matrix operations, on the GPU (using Vulkan).
// The ngrid.h library can be inluded as a tool in any other other program. This "main" file is not necessary for it to work.

// Dependencies: standard library, plus the following custom headers (these are all header-only files, no .cpp required):
// angular.h, log.h, vkcontext.h, spirv_bin.h (or spirv_bin_precompiled.h)

// Please make sure Vulkan is also installed (vulkan/vulkan_core.h).

#include <ngrid.h>

int main() {
	NGrid::set_workgroup_size_1d(256);
	NGrid::set_workgroup_size_2d(16);

	// +=================================+   
	// | Introduction                    |
	// +=================================+
	Log::force("\nThis is a test output for the NGrid library, demonstrating its functionality.");
	Log::force("This library implements a vast collection of parallel compute operations on the GPU, using Vulkan.");

	Log::force("\n'NGrid' in this case is used as a typename for n-dimensional data-structures.");
	Log::force("\nAll values are by default stored as 32bit floating point");
	Log::force("Most of the following examples are printed only up to 3 decimals for better readability, but the actual values are still 32bit.");

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
	Log::force("For readability reasons, the following examples will use relatively small data structures");
	Log::force("with no more than 1-3 dimensions, but they could of course in theory hold millions of values.");

	// +=================================+   
	// | Constructors                    |
	// +=================================+
	Log::force("\n+=================================+\n| Constructors                    |\n+=================================+");
	Log::force("\nLet's start with a 10x10 2d matrix using the variadic template constructor: template<typename... Args> NGrid(Args... args);");
	Log::force("NGrid A(10,10);\nA.fill(5); // initializing all elements with a value of 5.0f");
	NGrid A(10, 10);
	A.fill(5);
	A.print("\nresult (A):");

	Log::force("\nLet's try a 3d array of size {4,4,2}:\nNGrid example_3d(4,4,2);\nexample_3d.fill_random_int(0,999);");
	NGrid example_3d(4, 4, 2);
	example_3d.fill_random_int(0, 999);
	example_3d.print("\nresult (example_3d):");

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

	// +=================================+   
	// | Assignment                      |
	// +=================================+
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

	// +=================================+   
	// | getters & setters               |
	// +=================================+
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
	Log::force("x = as_vector[0];\nresult: x = ", x, " (again, if everything works as intended, this value should be the same as above)");

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

	// +=================================+   
	// | Fill / Initialize               |
	// +=================================+
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

	// +=================================+   
	// | Neural Net Weight Initialization|
	// +=================================+
	Log::force("\n+=================================+\n| Neural Net Weight Initialization|\n+=================================+");
	Log::force("Several methods are implemented to serve as a good starting point for weight initialization of neural networks,");
	Log::force("depending on the layer's activation function and the number of input and output connections:");
	Log::force("    void weightinit_tanh_normal(uint32_t fan_in, uint32_t fan_out);   // 'Xavier' tanh normal method (Xavier Glorot, Yoshua Bengio)");
	Log::force("    void weightinit_tanh_uniform(uint32_t fan_in, uint32_t fan_out);  // 'Xavier' tanh uniform method (Xavier Glorot, Yoshua Bengio)");
	Log::force("    void weightinit_sigmoid(uint32_t fan_in, uint32_t fan_out);       // 'Xavier' sigmoid uniform method (Xavier Glorot, Yoshua Bengio)");
	Log::force("    void weightinit_relu(uint32_t fan_in);                            // 'Kaiming He' method for ReLU activation");
	Log::force("    void weightinit_elu(uint32_t fan_in);                             // 'Kaiming He' method for ELU activation");
	Log::force("\nHere's an example for sigmoid activation (with 25 inputs and outputs for each neuron):\nNGrid I(5,5);\nI.weightinit_sigmoid(25,25);");
	NGrid I(5, 5);
	I.weightinit_sigmoid(25, 25);
	I.print("\nresult: initialized weights (I):");

	// +=================================+
	// | Distribution Properties         |
	// +=================================+
	Log::force("\n+=================================+\n| Distribution Properties         |\n+=================================+");
	Log::force("There currently are methods to retrieve the minimum, maximum, max absolute, mean, median, variance, standard deviation, kurtosis and skewness");
	Log::force("As an example, let's take a 1d matrix 'J' with 50 random elements from a Gaussian normal distribution with a mean of 2.0 and a standard deviation of 1.0:");
	Log::force("NGrid J(50);\nJ.fill_random_gaussian(2,1);");
	NGrid J(50);
	J.fill_random_gaussian(2, 1);
	J.print("Source vector (J):");
	Log::force("float_t x = J.min();     // result: x = ", J.min());
	Log::force("float_t x = J.max();     // result: x = ", J.max());
	Log::force("float_t x = J.maxabs();  // result: x = ", J.maxabs());
	Log::force("float_t x = J.mean();    // result: x = ", J.mean(), " (we expect a result close to 2.0, according to the initialization method of J)");
	Log::force("float_t x = J.median();  // result: x = ", J.median());
	Log::force("float_t x = J.var();     // result: x = ", J.var(), " (we expect a result close to 1.0 (stdev squared)");
	Log::force("float_t x = J.stdev();   // result: x = ", J.stdev(), " (we expect a result close to 1.0, according to the initialization method of J)");
	Log::force("float_t x = J.kurt();    // result: x = ", J.kurt());
	Log::force("float_t x = J.skew();    // result: x = ", J.skew());

	// +=================================+   
	// | Elementwise Arrithmetics        |
	// +=================================+
	Log::force("\n+=================================+\n| Elementwise Arrithmetics        |\n+=================================+");
	Log::force("Let's take the following 3x3 example NGrid:\nNGrid K(3,3);\nK.fill_random_int(1,9);");
	NGrid K(3, 3);
	K.fill_random_int(1, 9);
	K.print("\nNGrid K:");

	Log::force("\nWe can calculate the sum of all elements:\nfloat_t sum_K = K.sum(); // result: sum_K = ", K.sum());
	Log::force("\nSimilarly, we can calculate the product of all elements:\nproduct_K = K.product(); // result: product_K = ", K.product());
	Log::force("Note: this can quickly result in very large numbers, exceeding the limits of 32bit floats, resulting in +INF or -INF.");

	Log::force("\nWe can easily add or substract a value elementwise (or multiply with or divide by a value elementwise), for example:\nK = K + 1;");
	K = K + 1;
	K.print("\nupdated result (K): ");

	Log::force("\nThe prefix and postfix increment/decrement operators are also implemented; for example:\nK++;");
	K++;
	K.print("\nupdated result (K): ");

	Log::force("\nThe operator+=, operator-=, operator*= and operator/= for in-place addition/substraction/multiplication/division");
	Log::force("are also implemented, for example:\nK -= 3.0f; ");
	K -= 3.0f;
	K.print("\nupdated result (K): ");

	Log::force("\nSimilarly, the modulo operator (% and %=) calculates elementwise the remainder of the division by a scalar value");

	Log::force("\nLet's now take a second NGrid called 'L':\nNGrid L(3,3);\nL.fill_random_int(-9,0);");
	NGrid L(3, 3);
	L.fill_random_int(-9, 0);
	L.print("L:");
	Log::force("\nNow we can add both NGrids together (or substract), for example:\nNGrid M = K + L;");
	NGrid M = K + L;
	M.print("\nresult (M): ");
	Log::force("\nUsing the operator += should lead to the same result:\n K += L;");
	K += L;
	K.print("\nupdated result (K):");
	Log::force("\nHere's an example for the elementwise *= operation:\nK *= 3;");
	K *= 3;
	K.print("\nupdated result (K):");

	// +=================================+   
	// | Matrix Multiplication           |
	// +=================================+
	Log::force("\n+=================================+\n| Matrix Multiplication           |\n+=================================+");
	Log::force("\nAbove, we've already seen an example of elementwise multiplication by a 'scalar' value. However, there are other types of matrix multiplication:");

	Log::force("\nIf all elements of 'this' are supposed to be multiplied (or divided) elementwise by the corresponding elements of 'other',");
	Log::force("this is a Hadamard product(or division), for example:\nNGrid C = A.Hadamard_product(B);\nNGrid D = A.Hadamard_division(B);");

	Log::force("\nThe next type of multiplication is the scalar product (aka 'dot product'),");
	Log::force("i.e.the sum of elementwise multiplication of to vectors of matrices of equal dimensions, for example:");
	Log::force("float_t c = A.scalar_product(B);");

	Log::force("\nAnother type of multiplication is the 'matrix product' in the conventional sense.");
	Log::force("The result is a square matrix in case of the multiplication of the multiplication of two 2d matrices");
	Log::force("or a size {1} 'scalar' matrix (i.e. holding a single element) in case of multiplication of two 1d vectors, in which case it's the same as the scalar product.");
	Log::force("Note: the 'inner' dimensions must match, e.g. A{m,n} * B{n,o}; Example:\nNGrid O(3,5); O.fill_random_int(0,9);\nNGrid P(5,3); P.fill_random_int(0,9)");
	NGrid O(3, 5); O.fill_random_int(0, 9); O.print("\nmatrix O:");
	NGrid P(5, 3); P.fill_random_int(0, 9); P.print("\nmatrix P:");
	(O * P).print("\nresult of multiplication O * P:");
	Log::force("Note: the alias method NGrid::matrix_product() can be used alternatively instead of the * operator for clarity, demonstrating which multiplication method is used");
	Log::force("The *= operator is also supported, e.g. A *= B;");
	Log::force("In case of division, A / B is synonymous with A * B.inverse(); A /= B is synonymous with A *= B.inverse();");

	// +=================================+   
	// | Exponentiation & Logarithm      |
	// +=================================+
	Log::force("\n+=================================+\n| Exponentiation & Logarithm      |\n+=================================+");
	Log::force("\nThis library supports elementwise exponentiation to the power of a specified exponent");
	Log::force("with the method NGrid::pow(float_t exponent) or the operators ^ and ^= .");
	Log::force("Moreover, an NGrid object can be raised elementwise to the powers of the corresponding element of second");
	Log::force("NGrid by using the methods pow(NGrid& other) or ^(other). Examples:");
	NGrid Q(3, 10);
	Q.fill_random_int(1, 5);
	Q.print("\nLet's start with an NGrid Q(3,10) filled with random integer within range [1,5]:");
	(Q ^ 2).print("\nQ^2:");
	(Q.pow(-3)).print("\nQ.pow(-3):");
	Log::force("\nWe can also apply the elementwise square root, log or exp.");
	(Q.sqrt()).print("\nQ.sqrt():");
	(Q.log()).print("\nQ.log() [this is the natural log by default, i.e. base e=2.718282]:");
	(Q.log(10)).print("\nQ.log(10) [example for logarithm with base 10:");
	(Q.exp()).print("\nQ.exp():");

	// +=================================+   
	// | Rounding                        |
	// +=================================+
	Log::force("\n+=================================+\n| Rounding                        |\n+=================================+");
	Log::force("\nLet's consider the following NGrid R(10) filled with floating point numbers:");
	NGrid R(10);
	R.fill_random(-9, 9);
	R.print("R =");
	R.round().print("\nR.round() =");
	R.floor().print("\nR.floor() =");
	R.ceil().print("\nR.ceil() =");
	R.abs().print("\nR.abs() =");

	// +=================================+   
	// | Min, Max                        |
	// +=================================+
	Log::force("\n+=================================+\n| Min, Max                        |\n+=================================+");
	Log::force("\nWe've already seen how min() and max() can be used to find the minimum or maximum of ALL elements of the NGrid.");
	Log::force("min(value) and max(value) can be used for elementwise comparison. The individual elements are assigned to whichever is lower");
	Log::force("(or higher in the case of max(value)) out of the original element versus the specified value.");
	NGrid X(7, 5);
	X.fill_random_int(-999, 999);
	X.print("\nsource NGrid X:");
	X.min(0).print("\nX.min(0):");
	X.max(0).print("\nX.max(0):");
	Log::force("\nWe can also compare elementwise with a second NGrid and reassign the min or max between both (elementwise).");
	NGrid Y(7, 5);
	Y.fill_random_int(-999, 999);
	Y.print("\NGrid Y:");
	X.min(Y).print("\nX.min(Y):");
	X.max(Y).print("\nX.max(Y):");

	// +=================================+   
	// | Trigonometry Functions          |
	// +=================================+
	Log::force("\n+=================================+\n| Trigonometry Functions          |\n+=================================+");
	Log::force("\nThis library supports the following trigonometry functions:");
	Log::force("  (1) cos(), sin(), tan()              with optional argument for the source angular unit (default: RAD)");
	Log::force("  (2) acos(), asin(), atan()           with optional argument for the target angular unit (default: RAD)");
	Log::force("  (3) cosh(), sinh(), tanh()");
	Log::force("  (4) acosh(), asinh(), atanh()");
	X = X.reshape(10);
	X.fill_random_int(0, 360);
	X.print("\nExample for source angles: X =");
	X.sin(DEG).print("\nX.sin(DEG) =");

	// +=================================+   
	// | Find, Replace                   |
	// +=================================+
	Log::force("\n+=================================+\n| Find, Replace                   |\n+=================================+");
	Log::force("\nThis library supports several find/replace methods.");
	Log::force("Like most other methods, these are also non - destructive and return the result rather than modifying the source itself");
	X = X.reshape(15, 15);
	X.fill_random_int(0, 5);
	X.print("\nLet's consider the following source matrix X:");
	Log::force("\nuint32_t n = X.find(2) can e.g. be used to count the number of occurences of the value '2'. Result: n = ", X.find(2));
	X.replace(0, 1).print("\nHere's an example replacing all zeros with ones: X.replace(0,1) =");
	Log::force("\nWe can also replace based on a 'condition map' (i.e. a boolean NGrid of the same size)");
	X.replace_if(X % 2 == 0, 0).print("Here's an example with a condition to set all even numbers to zero: X.replace_if(X % 2 == 0, 0) =");
	Log::force("\nNote: instead of replacing with a single value, we can also replace 'this' NGrid elementwise with 'other' NGrid,");
	Log::force("based on a condition map: replace_if(condition_map, replacing_map)");
	Log::force("\nThe sign() method can be used to return an NGrid of equal dimensions, with -1 for all negative values, 0 for all zeros and +1 for all positive values");
	X = X.reshape(5, 5);
	X.fill_random_int(-9, 9);
	X.print("\nExample: X =");
	X.sign().print("\nX.sign() = ");
	Log::force("\nThe methods isnan() and isinf() return a boolean NGrid of equal dimensions with 1.0 for each NAN ('not a number') or each pos/neg INFinity, respectively,");
	Log::force("and 0.0 for all valid numbers");
	X.fill_random_int(-3, 5);
	X = X.sqrt();
	X.print("\nLet's take an NGrid with a few NANs as an example. X = ");
	X.isnan().print("\nX.isnan() =");

	// +=================================+   
	// | Data Preprocessing              |
	// +=================================+
	Log::force("\n+=================================+\n| Data Preprocessing              |\n+=================================+");
	X = X.reshape(30);
	X.fill_range(0, 1);
	Log::force("\nLet's take the following 1d-NGrid X with shape X.get_shapestr() = ", X.get_shapestring(), ", X.mean() = ", X.mean(), " and X.stdev() = ", X.stdev(), ":");
	X.print();

	Log::force("\nWe have the following scaling options:");
	X.scale_minmax(10, 20).print("\nMinMax-Scaling: shift the minimum and stretch/compress the range to fit within [min,max], e.g.: X.scale_minmax(10,20) = ", 2);
	X.scale_mean().print("\nShift to zero mean, stretch/compress to range to fit within [-1,1], e.g. X.scale_mean() = ", 2);
	X.scale_zscore().print("\nShift to zero mean, strech/compress to match unit-variance (i.e. z_score = 1.0 = default) or the specified z_score, e.g. X.scale_zscore(1.0f) = ", 2);
	Log::force("Let's verify the standard deviation of this result (it should be ~1.0): X.scale_score(1).stdev() = ", X.scale_zscore().stdev());

	Log::force("\nWe have the following options for outlier treatment:");
	X.outliers_clamp_minmax(10, 20).print("\nClamp the values of the array within range [min,max], e.g. X.outliers_clamp_minmax(10,20) = ");
	X.outliers_clamp_zscore(1.0f).print("\nClamp the values within a specified z-score, e.g. in order to limit to +/- 1 standard deviation: X.outliers_clamp_score(1.0f) = ", 2);
	X.outliers_mean_imputation(1.0f).print("\nSet all values that exceed the specified z-score by the arrithmetic mean of all values, e.g. X.outliers_mean_imputation(1.0f) = ", 2);
	X.outliers_value_imputation(1.0f, 0.0f).print("\nSet all values that exceed the specified z-score with the specified value, e.g. replace outliers that exceed +/- 1 sigma by 0: X.outliers_value_imputation(2.0f, 0.0f) = ", 2);
	Log::force("\nNGrid::recover(): 'Recovers' invalid data by replacing +INF with FLOAT_MAX, -INF with -FLOAT_MAX and NAN with 0.");
	Log::force("To test this, let's set some of the values intentionally to NAN or INF:");
	Y = X.replace_if(X % 3 == 0, std::numeric_limits<float>::quiet_NaN());
	Y = Y.replace_if(X % 4 == 0, std::numeric_limits<float>::infinity());
	Y *= -1;
	Y.print("Y = ");
	Y.recover().print("Y.recover() =");

	// +=================================+   
	// | Activation Functions            |
	// | (with Derivatives)              |
	// +=================================+
	Log::force("\n+=================================+\n| Activation Functions            |\n+=================================+");
	Log::force("\nThis library has support for activation functions (for neural networks):");
	Log::force("    RELU:    rectified linear unit");
	Log::force("    LRELU:   'leaky' rectified linear unit");
	Log::force("    ELU:     exponential linar unit");
	Log::force("    LELU:    leaky exponential linear unit");
	Log::force("    SIGMOID: sigmoid (=logistic)");
	Log::force("    TANH:    hyperbolic tangent (tanh), with angular unit radians");
	Log::force("    IDENT:   identity function");
	Log::force("The functions can either be used directly: NGrid::sigmoid(), NGrid::tanh() ... [or NGrid_sigmoid_drv(), ... etc. for their derivatives]");
	Log::force("or NGrid::activation(ActFunc) / NGrid::derivative(ActFunc) can be used for more flexibility (passing the used function as an argument makes changes easier).");
	X = X.reshape(5, 5);
	X.fill_random_gaussian();
	X.print("\nfor example for X =");
	X.sigmoid().print("\n... X.sigmoid() =");
	X.activation(SIGMOID).print("\n... which is the same as saying X.activation(SIGMOID) = ");

	// +=================================+   
	// | Elementwise Comparison          |
	// +=================================+
	Log::force("\n+=================================+\n| Elementwise Comparison          |\n+=================================+");
	Log::force("\nThis library supports operators for elementwise comparison: >, >=, ==, !=, <, <=");
	Log::force("\n These comparisons can be done by comparing with a single value, or elementwise with the corresponding elements of 'other'");
	Log::force("The return value is a 'boolean' NGrid (1.0f for true, 0.0f for false)");
	X.fill_random_int(0, 9);
	X.print("\nFor example, for X =");
	(X > 5).print("... X > 5 = ");
	Log::force("\nAccordingly, logical operators are supported: &&, || to compare with 'other' or a single boolean value, as well as ! to toggle each element to its boolean opposite");
	Log::force("For example:");
	X = X.reshape(40); X.fill_random_binary(); X.print("\nX =");
	Y = Y.reshape(40); Y.fill_random_binary(); Y.print("\nY =");
	(X && Y).print("\nX && Y = ");
	(X || Y).print("\nX || Y = ");
	(!X).print("\n!X = ");

	// +=================================+   
	// | Advanced Matrix Operations      |
	// +=================================+
	Log::force("\n+=================================+\n| Advanced Matrix Operations      |\n+=================================+");
	X = X.reshape(3, 3); X.fill_random_int(0, 9);
	X.print("\nWe can 'flatten' any multidimensional NGrid to a 1d vector, e.g. for X =");
	X.flatten().print("\nX.flatten() =");

	X.reshape({ 4, 5 }, 0.0f).print("We can reshape/resize whilst maintaining any preexisting values, e.g. X.reshape(4,5) = ");
	Log::force("Note: this method is also non-destructive, i.e. to actually assign the new shape we have to write X = X.reshape(...)");

	Y = Y.reshape(3, 3); Y.fill_random_int(0, 9);
	Y.print("\nLet's take a second NGrid Y =");
	X.concatenate(Y, 0).print("\nLet's now concatenate the original NGrid X with Y along the row axis: X.concatenate(Y, 0) = ");
	X.concatenate(Y, 1).print("\n... or alternatively along the column axis: X.concatenate(Y, 1) =");
	X.concatenate(Y, 2).print("\n... or stacked along z axis: X.concatenate(Y,2)");

	X = X.reshape(10, 10); X.fill_random_int(1, 3);
	X.padding(2, 0).print("\nThe method NGrid::padding(amount,init_value) can be used to 'pad' e.g. to be used before convolution");
	X.print("\ne.g. X = ");
	X.padding(2, 0).print("\nX.padding(2,0) = ");
	Y = Y.reshape(3, 3); Y.fill_random_binary();
	Y.print("\nExample for a filter kernel: Y =");
	Log::force("\nThe method NGrid::convolution(kernel,padding_amount,padding_value) also has a built-in padding-option:");
	X = X.convolution(Y, 2, 0);
	X.print("\nX.convolution(Y,2,0) =");

	Log::force("\nThis library supports the following pooling options: max, min, maxabs. The methods each take an argument for the pooling window and for the stride.");
	Log::force("For example, let's perform a max pooling operation on the result of the convolution with a {2,2} window and a {2,2} stride (i.e. no overlap between stride steps):");
	X.pool_max({ 2,2 }, { 2,2 }).print("\nX.pool_max({2,2},{2,2}) = ");

	X = X.reshape(20); Y = Y.reshape(20);
	X.fill_range(0, 3); Y.fill_random_int(-2, 2); X = X + Y; // simulating values of a time series
	X.print("\nLet's now take the following values of a time series: X =");
	X.stationary().print("\nHere's how this series can be made stationary: e.g. for first order differencing: X.stationary(1) = ");
	X.stationary_log().print("\nAlternatively, for first order logarithmic stationary transformation (log base 10): X.stationary_log(1,10) = ");
	Log::force("\nThe Dickey-Fuller test (e.g. with p <= 0.05) can be used to confirm that a series is indeed stationary, e.g. X.stationary_log(1,10).Dickey_Fuller() = ", X.stationary_log(1, 10).Dickey_Fuller(), " (should be <0.05)");

	X.fill_random_int(0, 999);
	Log::force("\nThe method NGrid::sort() can be used for ascending or descending order sorting (the implementation is based on the even-odd sort ('brick sort') algorithm):");
	X.print("unsorted X =");
	X.sort(true).print("ascending: X.sort(true) = ");
	X.sort(false).print("descending: X.sort(false) = ");

	Log::force("\nThis library supports matrix transpose:");
	X = X.reshape(4, 7); X.fill_random_int(0, 9);
	X.print("\n... for X =");
	X.transpose().print("\nX.transpose() =");

	Log::force("\nThis library supports LU decomposition, which is the basis for calculation the matrix inverse.");
	X = X.reshape(7, 7); X.fill_random_int(-9, 9);
	X.print("\n... for X =");
	Log::force("\nuint32_t swap_count = X.lu().swap_count();");
	auto lu_result = X.lu();
	Log::force("\nswap_count = ", lu_result.swap_count(), " row swaps have been performed.");
	lu_result.L.print("\nresult for lower triagonal matrix L =");
	lu_result.U.print("\nresult for upper triagonal matrix U =");
	Log::force("\nThe source matrix has a 'rank' (=number of linearly independent rows) of NGrid::rank(U) = ", NGrid::rank(lu_result.U), " (Note: this is a static method)");
	Log::force("uint32_t r = X.rank() can be used alternatively when U isn't available (it will then be calculated internally, adding some overhead)");
	P.print("\nresult for permutation matrix (row swaps) P =");
	Log::force("\nWe can optionally check if the matrix is invertible by using X.is_invertible() [result = ", X.is_invertible(), "];");
	Log::force("This method runs LU decomposition internally and checks if the U matrix has no zeros on its diagonal.");
	Log::force("Alternatively - if the U matrix is already known like in the case above - we can slightly reduce overhead by reusing it in the static method NGrid::is_invertible(const NGrid& U):");
	Log::force("bool result = NGrid::is_invertible(U); // result = ", NGrid::is_invertible(lu_result.U));

	X.inverse().print("\nWe can calculate the matrix inverse as follows: X.inverse() =");

	Log::force("\nAnother approach to obtain the inverse is by augmenting the matrix by the identity matrix and then calculate the reduced row echelon form (achieved via Gauss-Jordan elimination).");
	Log::force("The 'right' part of the RREF then becomes the inverse:");
	NGrid X_ident(X.get_shape()[0], X.get_shape()[1]); X_ident.fill_identity();
	X_ident.print("\nX_ident = ");
	X.concatenate(X_ident, 1).print("\nBEFORE the operation, the augmented matrix [X|X_ident] would look like this:  X.concatenate(X_ident, 1) = ");
	X.rref(X_ident).augmented().print("\nThe concatenation (=augmented matrix) above was only for clarification; we don't have to do this manually. To get the RREF we can simply write directly: X.rref(X_ident).augmented() = ");
	Log::force("\nFrom this result, we could extract the inverse by using the subgrid method. However, this is unnecessary, because it's directly available as a struct member:");
	X.rref(X_ident).solution.print("\nX.rref(X_ident).solution =");
	X.rref(X_ident).coeffs.print("\nX.rref(X_ident).coeffs = ");
	Log::force("\nThe method NGrid::rref() can also be used to get the solution of a system of linear equations.");
	Y = Y.reshape(X.get_shape()[0]); Y.fill_random_int(-9, 9);
	Y.print("\nLet's say we augment X by the following vector Y:");
	X.rref(Y).augmented().print("\nThe RREF then becomes: X.rref(Y).augmented() =");
	X.rref(Y).solution.print("Or directly for the solution: X.rref(Y).solution = ");

	Log::force("\nThis library supports QR decomposition: auto qr_result = X.qr();");
	X = X.reshape(6, 6); X.fill_random_int(1, 9);
	X.print("\n... for X =");
	auto qr_result = X.qr();
	qr_result.Q.print("\nQ =");
	qr_result.R.print("\nR =");
	qr_result.Tau.print("\nTau = ");
	qr_result.V.print("\nV =");

	Log::force("\nLet's verify that these results are actually correct:");

	(qr_result.Q.transpose() * qr_result.Q).print("\nqr_result.Q.transpose() * qr_result.Q should give us the identity matrix:");
	(qr_result.Q * qr_result.Q.transpose()).print("\nqr_result.Q * qr_result.Q.transpose() should also give us the identity matrix (if Q is indeed an orthogonal matrix:");
	(qr_result.Q * qr_result.R).print("\nqr_result.Q * qr_result.R should be equal to the source matrix X (potentially with small rounding errors:");

	Log::force("\nThis library support Hessenberg transformation:");
	X.print("\nFor a test, let's reuse the same source matrix as above, i.e. X =");
	Log::force("\n... we get auto hess_result = X.hess() (or .. =qr(true)) with:");
	auto hess_result = X.hess();
	hess_result.R.print("\nhess_result.R = ");
	hess_result.Q.print("\nhess_result.Q = ");
	hess_result.Tau.print("\nhess_result.Tau = ");
	hess_result.V.print("\nhess_result.V =");

	Log::force("\nIf the result are correct and the source matrix is square (which it is), then Q*R*Q^T should give us the original matrix X.");
	(hess_result.Q * hess_result.R * hess_result.Q.transpose()).print("\nhess_result.Q * hess_result.R * hess_result.Q.transpose() =");

	(hess_result.Q * hess_result.Q.transpose()).print("\nQ * Q.transpose should give us the identity matrix:");

	Log::force("\nLet's now calculate the eigenvalues of X: auto eigen_result = X.eigen();");
	X.eigen(10000, 1e-08).print("\n Eigen values: ");

	// +=================================+   
	// | Statistics                      |
	// +=================================+

	// ... TODO

	// +=================================+   
	// | Miscellaneous                   |
	// +=================================+

	// ... TODO
}

