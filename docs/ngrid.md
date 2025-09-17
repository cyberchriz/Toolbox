# NGrid Library (`ngrid.h`)

## 📜 Summary
The `ngrid.h` library provides a powerful n-dimensional data structure, the `NGrid` class, designed for high-performance mathematical and scientific computing. It leverages the power of GPU acceleration via the Vulkan API to perform a vast array of parallel computations, from basic arithmetic to complex matrix operations, statistical analysis, and neural network functions.

_Note: This library relies on a custom Vulkan wrapper ([`vkcontext.h`](../include/vkcontext.h)) for managing the GPU context and assumes that compute shader binaries (`spirv_bin.h`) are available (auto-generated into the project output folder by CMake, using the provided CMakeLists.txt in this repository) or [precompiled](../include/spirv_bin_precompiled.h)._

---

## ✨ Features
- **N-Dimensional Data**: Natively create and manipulate float type tensors of any shape and dimension.
- **GPU Accelerated**: All mathematical operations are executed on the GPU via Vulkan compute shaders for maximum parallelism and speed.
- **Asynchronous Execution**: all calculations execute asychronously on the GPU, whilst the host proceeds with its work, without waiting for intermediate results;<br>synchronization is only enforced when final results are read back to the host.
- **Rich Mathematical Toolkit**: An extensive set of operations including linear algebra, statistical functions, random number generation, and element-wise calculations.
- **Neural Network Support**: Includes specialized functions for weight initialization, activation functions, and their derivatives.
- **Dynamic Manipulation**: Easily reshape, concatenate, pad, sort, flatten and perform complex transformations like convolution, inverse, transpose, pooling,<br>stationary transformation, QR decomposition, LU decomposition, RREF, and more ...
- **Seamless Interoperability**: Natively supports type conversion with the `CGrid` class for handling complex numbers.
- **Non-destructive Manipulation**: Most methods, except for fill or assignment operations, don't directly manipulate the NGrid source object,<br>thereby following mathematical intuition. For example a = b + x would manipulate a, but not b or x.

---

### 🛠️ Constructors & Destructors
Methods for creating and destroying `NGrid` instances.

| **Method**| **Description**|
| :--- | :--- |
| `NGrid()` | The default constructor initializes an empty array. |
| `NGrid(...)` | Parametric default constructor for multi-dimensional array, overload for variadic template. |
| `NGrid(const std::vector<uint32_t>& shape)` | Parametric default constructor for multi-dimensional array, overload for `std::vector`. |
| `NGrid(std::initializer_list<uint32_t> shape)`| Parametric default constructor for multi-dimensional array, overload for `std::initializer_list`. |
| `NGrid(const std::vector<float_t>& source_vector)` | Constructs a 1D array and directly fills it with the contents of a `std::vector<float_t>`.|
| `NGrid(NGrid&& other) noexcept`| Move constructor. |
| `NGrid(const NGrid& other)`| Copy constructor. |
| `~NGrid()` | Destructor. |

---

### 🔀 Assignment
Methods for assigning values to an `NGrid` instance.

| **Method**| **Description**|
| :--- | :--- |
| `operator=` | Copy assignment. |
| `operator=(NGrid&& other) noexcept`| Move assignment. |
| `operator=` | Assigns the contents of a `std::vector<float_t>` or a `float_t*` pointer to the grid. These are aliases for the `set()` methods. |

---

### 📥 Getters & Setters
Methods for getting and setting elements or properties of the `NGrid` instance.

| **Method**| **Description**|
| :--- | :--- |
| `set(...)` | Destructive setter methods that modify the `NGrid` instance itself, then return a reference to `this`. Overloads are provided to set a single value by index, or to copy data from a `std::vector`, a `float_t` pointer, or another `NGrid` instance with optional offset and element count. |
| `get(...)` | Non-destructive getter methods that return a copy of the requested data. Overloads are provided to retrieve a single value by index, a subset of elements, or the entire underlying data as a `std::vector<float_t>`. |
| `get_buffer()` | Returns a reference to the underlying `Buffer<float_t>` containing the array data. |
| `get_shape_buffer()` | Returns a reference to the underlying `Buffer<uint32_t>` containing the shape data. |
| `get_dimensions()` | Returns the number of dimensions of the array. |
| `get_size(uint32_t dimension)` | Returns the size of the specified dimension. |
| `get_elements()` | Returns the total number of elements in the array. |
| `get_shape()` | Returns the shape of the array as a `const std::vector<uint32_t>&`. |
| `rows()` | Returns the number of rows (size of the first dimension). |
| `cols()` | Returns the number of columns (size of the second dimension). |
| `get_shapestring()` | Returns the shape of the array as a formatted string, e.g., `{2,3,4}`. |
| `subgrid(const std::vector<uint32_t>& source_offset, const std::vector<uint32_t>& subgrid_shape)`| Slices a subarray out of the parent array. |
| `subgrid(const std::initializer_list<uint32_t> source_offset, const std::initializer_list<uint32_t> subgrid_shape)`| Slices a subarray out of the parent array. |
| `set(const NGrid& other, const std::vector<uint32_t>& target_origin_offset)`| Sets a subgrid by copying data from another NGrid using a multidimensional offset. |
| `set(const NGrid& other, const std::initializer_list<uint32_t>& target_origin_offset)`| Sets a subgrid by copying data from another NGrid using a multidimensional offset. |
| `operator=` | The assignment operator, aliased to `set()`.|

---

### 🔄 Fill
Destructive methods that modify the `NGrid` instance by filling it with specific values.

| **Method**| **Description**|
| :--- | :--- |
| `fill(float_t value)` | Fills the entire grid with a single `float_t` value. |
| `fill_zero()` | Fills the grid with zeros. |
| `fill_identity()` | Fills a square grid with an identity matrix. |
| `fill_random_gaussian(mu, sigma)`| Fills the grid with values drawn from a Gaussian (Normal) distribution. |
| `fill_random_uniform(min, max)` | Fills the grid with values drawn from a uniform distribution. |
| `fill_random_uniform_int(min, max)` | Fills the grid with random integer values from a uniform distribution. |
| `fill_random(min, max)`| Alias for `fill_random_uniform()`. |
| `fill_random_int(min, max)`| Alias for `fill_random_uniform_int()`. |
| `fill_random_binary(ratio)` | Fills the grid with either 0.0 or 1.0 based on the specified ratio. |
| `fill_random_sign(ratio)` | Fills the grid with either -1.0 or 1.0 based on the specified ratio. |
| `fill_range(start, step)` | Fills the grid with a sequence of numbers, starting at `start` and increasing by `step`<br>(`step` can be negative). |
| `fill_dropout(ratio)` | Fills a specified ratio of elements with zeros, leaving others unchanged. |
| `fill_index()` | Fills the grid with the flat index of each element. |

---

### ➕ Static Constructors
Static methods that create and return new `NGrid` instances.

| **Method**| **Description**|
| :--- | :--- |
| `static NGrid identity(const uint32_t size)` | Creates a new square identity `NGrid` of the specified size. |

---

### 🧠 Neural Net Weight Initialization
Methods for initializing weights for neural network layers.

| **Method**| **Description**|
| :--- | :--- |
| `weightinit_tanh_normal(fan_in, fan_out)` | Initializes weights using a tanh normal distribution. |
| `weightinit_tanh_uniform(fan_in, fan_out)`| Initializes weights using a tanh uniform distribution. |
| `weightinit_sigmoid(fan_in, fan_out)` | Initializes weights suitable for a sigmoid activation function. |
| `weightinit_relu(fan_in)` | Initializes weights suitable for a ReLU activation function. |
| `weightinit_elu(fan_in)` | Initializes weights suitable for an ELU activation function. |

---

### 📊 Distribution Properties
Methods to analyze the statistical properties of the data.

| **Method**| **Description**|
| :--- | :--- |
| `min()` | Returns the minimum value in the array. |
| `max()` | Returns the maximum value in the array. |
| `maxabs()` | Returns the maximum absolute value in the array. |
| `mean()` | Returns the mean (average) of all elements. |
| `median()` | Returns the median of all elements. |
| `var(sample_var)`| Returns the variance of the elements. `sample_var=true` for sample variance, `false` for population variance. |
| `stdev()` | Returns the standard deviation. |
| `kurt(sample_kurt)` | Returns the kurtosis of the data. `sample_kurt=true` for sample kurtosis, `false` for population kurtosis. |
| `skew(sample_skew)` | Returns the skewness of the data. `sample_skew=true` for sample skewness, `false` for population skewness. |

---

### ➕ Addition
Methods for addition operations.

| **Method**| **Description**|
| :--- | :--- |
| `sum()` | Returns the sum of all array elements. |
| `operator+(float_t value)`| Returns a new `NGrid` with `value` added to each element. |
| `operator+(const NGrid& other)`| Returns a new `NGrid` that is the element-wise sum of this grid and another. |
| `operator++()` | Prefix increment. |
| `operator++(int)` | Postfix increment. |
| `operator+=(float_t value)` | Adds `value` to each element in-place. |
| `operator+=(const NGrid& other)` | Performs an element-wise addition with `other` in-place. |

---

### ➖ Subtraction
Methods for subtraction operations.

| **Method**| **Description**|
| :--- | :--- |
| `operator-(float_t value)`| Returns a new `NGrid` with `value` subtracted from each element. |
| `operator-(const NGrid& other)`| Returns a new `NGrid` that is the element-wise difference of this grid and another. |
| `operator--()` | Prefix decrement. |
| `operator--(int)` | Postfix decrement. |
| `operator-=(float_t value)` | Subtracts `value` from each element in-place. |
| `operator-=(const NGrid& other)` | Performs an element-wise subtraction with `other` in-place. |

---

### ✖️ Multiplication
Methods for multiplication operations.

| **Method**| **Description**|
| :--- | :--- |
| `product()` | Returns the product of all array elements. |
| `operator*(float_t factor)`| Returns a new `NGrid` with each element multiplied by `factor`. |
| `operator*=(float_t factor)`| Multiplies each element by `factor` in-place. |
| `operator*(const NGrid& other)`| Alias for `matrix_product()`. |
| `operator*=(const NGrid& other)`| Performs a matrix product with `other` in-place. |
| `scalar_product(const NGrid& other)`| Returns the scalar (dot) product of this grid and another. |
| `matrix_product(const NGrid& other)`| Returns a new `NGrid` that is the matrix product of this grid and another. |
| `Hadamard_product(const NGrid& other)`| Returns a new `NGrid` that is the element-wise product of this grid and another. |

---

### ➗ Division
Methods for division operations.

| **Method**| **Description**|
| :--- | :--- |
| `operator/(float_t divisor)`| Returns a new `NGrid` with each element divided by `divisor`. |
| `operator/=(float_t divisor)`| Divides each element by `divisor` in-place. |
| `Hadamard_division(const NGrid& other)`| Performs element-wise division. |
| `operator/(const NGrid& other)` | Returns a new `NGrid` that is the matrix product with the inverse of `other`. |

---

### 🔢 Modulo
Methods for modulo operations.

| **Method**| **Description**|
| :--- | :--- |
| `operator%=(float_t value)` | Performs element-wise modulo `value` in-place. |
| `operator%(float_t value)` | Returns a new `NGrid` with each element's modulo `value`. |

---

### 📈 Exponentiation & Logarithm
Methods for exponentiation and logarithm operations.

| **Method**| **Description**|
| :--- | :--- |
| `pow(float_t exponent)` | Returns a new `NGrid` with each element raised to the power of `exponent`. |
| `operator^(float_t exponent)` | Alias for `pow(exponent)`. |
| `operator^=(float_t exponent)`| Performs `pow(exponent)` in-place. |
| `pow(const NGrid& other)` | Returns a new `NGrid` with each element raised to the power of the corresponding element in `other`. |
| `operator^(const NGrid& other)`| Alias for `pow(other)`. |
| `sqrt()`| Returns a new `NGrid` with the square root of each element. |
| `log(base)` | Returns a new `NGrid` with the logarithm of each element. Default base is `e` (2.718...). |
| `exp()` | Returns a new `NGrid` with the exponent of each element. |

---

### 📐 Rounding
Methods for rounding values.

| **Method**| **Description**|
| :--- | :--- |
| `round(precision)`| Returns a new `NGrid` with each element rounded to the specified precision. |
| `floor()`| Returns a new `NGrid` with each element rounded down to the nearest integer. |
| `ceil()`| Returns a new `NGrid` with each element rounded up to the nearest integer. |
| `abs()` | Returns a new `NGrid` with the absolute value of each element. |

---

### ⬇️ Min, Max
Methods for element-wise minimum and maximum operations.

| **Method**| **Description**|
| :--- | :--- |
| `min(float_t value)`| Returns a new `NGrid` with each element set to the minimum of its current value and `value`. |
| `max(float_t value)`| Returns a new `NGrid` with each element set to the maximum of its current value and `value`. |
| `min(const NGrid& other)`| Returns a new `NGrid` with each element set to the minimum of its current value and the corresponding element in `other`. |
| `max(const NGrid& other)`| Returns a new `NGrid` with each element set to the maximum of its current value and the corresponding element in `other`. |

---

### 📏 Trigonometric Functions
Methods for trigonometric and hyperbolic functions.

| **Method**| **Description**|
| :--- | :--- |
| `cos(source_angle_unit)` | Calculates the cosine of each element. `source_angle_unit` specifies if input is in `RAD` (default) or `DEG`.|
| `sin(source_angle_unit)` | Calculates the sine of each element. `source_angle_unit` specifies if input is in `RAD` (default) or `DEG`.|
| `tan(source_angle_unit)` | Calculates the tangent of each element. `source_angle_unit` specifies if input is in `RAD` (default) or `DEG`.|
| `acos(result_angle_unit)` | Calculates the arc cosine of each element. `result_angle_unit` specifies if output should be in `RAD` (default) or `DEG`.|
| `asin(result_angle_unit)` | Calculates the arc sine of each element. `result_angle_unit` specifies if output should be in `RAD` (default) or `DEG`.|
| `atan(result_angle_unit)` | Calculates the arc tangent of each element. `result_angle_unit` specifies if output should be in `RAD` (default) or `DEG`.|
| `cosh()` | Calculates the hyperbolic cosine of each element. |
| `sinh()` | Calculates the hyperbolic sine of each element. |
| `tanh()` | Calculates the hyperbolic tangent of each element. |
| `acosh()` | Calculates the inverse hyperbolic cosine of each element. |
| `asinh()` | Calculates the inverse hyperbolic sine of each element. |
| `atanh()` | Calculates the inverse hyperbolic tangent of each element. |

---

### ⚖️ Elementwise Comparison
Methods for element-wise comparison. Results are a new `NGrid` with 1.0 for true and 0.0 for false.

| **Method**| **Description**|
| :--- | :--- |
| `operator>(...)`, `operator>=(...)`, etc. | Compares each element of the array to a `float_t` value or another `NGrid`. |

---

### 🎭 Activation Functions
Methods for common neural network activation functions.

| **Method**| **Description**|
| :--- | :--- |
| `activation(ActFunc)` | Applies the specified activation function to each element. |
| `derivative(ActFunc)` | Applies the specified activation function's derivative to each element. |
| `ident()` | Applies the identity function (f(x) = x). |
| `ident_drv()` | Applies the derivative of the identity function (f'(x) = 1). |
| `sigmoid()` | Applies the sigmoid activation function. |
| `sigmoid_drv()`| Applies the derivative of the sigmoid function. |
| `elu(alpha)` | Applies the ELU (Exponential Linear Unit) function. |
| `elu_drv(alpha)` | Applies the derivative of the ELU function. |
| `relu(alpha)` | Applies the ReLU (Rectified Linear Unit) function. `alpha=0` for true ReLU, `0.01` for leaky ReLU. |
| `relu_drv(alpha)` | Applies the derivative of the ReLU function. |
| `tanh_drv()`| Applies the derivative of the hyperbolic tangent function. |

---

### 🔍 Find, Replace
Methods for searching and replacing values.

| **Method**| **Description**|
| :--- | :--- |
| `replace(old_value, new_value)` | Replaces all occurrences of `old_value` with `new_value`. |
| `replace_if(condition_map, replacing_map)` | Replaces elements where `condition_map` is non-zero with the corresponding elements from `replacing_map`. |
| `replace_if(condition_map, replacing_value)` | Replaces elements where `condition_map` is non-zero with `replacing_value`. |
| `find(value)` | Returns the flat index of the first occurrence of `value`. |
| `sign()` | Returns a new `NGrid` with the sign of each element (-1, 0, or 1). |
| `isinf()` | Returns a new `NGrid` where each element is 1.0 if the corresponding source element is infinite, otherwise 0.0. |
| `isnan()` | Returns a new `NGrid` where each element is 1.0 if the corresponding source element is NaN, otherwise 0.0. |

---

### 📈 Outlier Treatment
Methods for handling outliers and invalid data.

| **Method**| **Description**|
| :--- | :--- |
| `outliers_clamp_minmax(min, max)` | Clamps the values of the array within the range `[min, max]`. |
| `outliers_clamp_zscore(z_score)` | Truncates outliers based on `z_score`. |
| `outliers_mean_imputation(z_score)`| Replaces outliers which exceed +/- `z_score`*sigma by the arithmetic mean. |
| `outliers_value_imputation(z_score, value)`| Replaces outliers which exceed +/- `z_score`*sigma by the specified `value`. |
| `recover()` | 'Recovers' invalid data by replacing `+INF` with `FLOAT_MAX`, `-INF` with `-FLOAT_MAX`, and `NAN` with 0. |

---

### ⚖️ Scaling
Methods for normalizing and scaling data.

| **Method**| **Description**|
| :--- | :--- |
| `scale_minmax(range_from, range_to)`| Shifts the minimum and stretches/compresses the range to fit within `[range_from, range_to]`. |
| `scale_mean()` | Shifts to zero mean and stretches/compresses to not exceed range `[-1,1]`. |
| `scale_zscore(z_score)`| Shifts to zero mean and stretches/compresses to match unit-variance (i.e., `z_score = 1.0` = default) or the specified `z_score`. |
| `scale_undo()`| Undoes the last scaling operation. |
| `scale_undo(const NGrid& scaling_reference)`| Undoes the scaling operation based on a reference `NGrid`. |

---

### 🧠 Advanced Matrix Operations
Methods for complex data transformations.

| **Method**| **Description**|
| :--- | :--- |
| `flatten()` | Returns a new `NGrid` with all dimensions collapsed into a single dimension. |
| `reshape(new_shape, ...)` | Returns a new `NGrid` with the specified new shape. Overloads exist for `std::vector`, `std::initializer_list`, and variadic templates. |
| `concatenate(other, axis)` | Concatenates this `NGrid` with another along the specified `axis`. |
| `padding(amount, init_value)` | Adds a specified amount of padding to all sides of the array. |
| `stationary(degree)`| Applies the stationary transform to the `NGrid` data for time-series analysis. |
| `stationary_log(degree, log_base)`| Applies the log-stationary transform. |
| `sort(ascending)` | Returns a new `NGrid` with the elements sorted. |
| `pool_max(window_shape, stride_shape)` | Performs max pooling on the array. |
| `pool_maxabs(window_shape, stride_shape)`| Performs max-absolute pooling on the array. |
| `pool_min(window_shape, stride_shape)` | Performs min pooling on the array. |
| `pool_mean(window_shape, stride_shape)` | Performs mean pooling on the array. |
| `convolution(kernel, padding_amount, padding_value)`| Performs a 2D convolution with a specified kernel. |
| `transpose(target_axis_order)`| Transposes the axes of the array. |
| `mirror(mirror_axes)` | Mirrors the array along the specified axes. |
| `remap(target_index_map)` | Creates a new `NGrid` by remapping elements using an index map. |

---

### ➕ Linear Algebra
Methods for linear algebra operations.

| **Method**| **Description**|
| :--- | :--- |
| `qr(hessenberg)`| Performs QR decomposition on the `NGrid`. |
| `hess()` | Performs Hessenberg decomposition. |
| `eigen(max_iterations_multiplier, tolerance)`| Calculates the eigenvalues and eigenvectors of a matrix. Returns a `CGrid` object. |
| `lu()` | Performs LU decomposition. |
| `l_inverse()`| Calculates the inverse of the L matrix from an LU decomposition. |
| `u_inverse()`| Calculates the inverse of the U matrix from an LU decomposition. |
| `inverse()` | Calculates the inverse of the `NGrid` matrix. |
| `static inverse(LUP)`| Calculates the inverse from a given `LUresult` struct. |
| `is_invertible()`| Checks if the `NGrid` matrix is invertible. |
| `static is_invertible(U)` | Checks if a given matrix `U` is invertible. |
| `rref(augment)` | Calculates the reduced row echelon form of the matrix. |
| `determinant()`| Calculates the determinant of the matrix. |
| `rank()` | Returns the rank of the matrix. |
| `static rank(U)` | Returns the rank of a given matrix `U`. |
| `static rank(LUP)` | Returns the rank from a given `LUresult` struct. |
| `diagonal()` | Returns a 1D `NGrid` containing the diagonal elements. |

---

### 📊 Statistics
Methods for statistical analysis.

| **Method**| **Description**|
| :--- | :--- |
| `regression(other, sample, degree)`| Performs a linear or polynomial regression of this `NGrid` against `other`. |
| `Dickey_Fuller()`| Performs the Dickey-Fuller test for stationarity. |
| `Engle_Granger(other)` | Performs the Engle-Granger test for cointegration. |
| `covariance(other)`| Calculates the covariance between this `NGrid` and another. |

---

### 🎁 Miscellaneous
Utility and configuration methods.

| **Method**| **Description**|
| :--- | :--- |
| `print(...)` | Prints a formatted representation of the grid to the console. |
| `set_workgroup_size_1d(size)` | Sets the default Vulkan workgroup size for 1D dispatches. |
| `set_workgroup_size_2d(size)` | Sets the default Vulkan workgroup size for 2D dispatches. |
| `get_tasks_finished_semaphore_counter()` | Returns the value of the semaphore counter. |
| `set_tasks_finished_semaphore_counter(value)`| Sets the value of the semaphore counter. |
| `operator CGrid()`| Explicit type conversion to `CGrid`. |
| `flat_index(multi_index)` | Returns the 1D flat index from a multidimensional index. |