# NGrid Library (`ngrid.h`)

## 📜 Summary
The `ngrid.h` library provides a powerful n-dimensional data structure, the `NGrid` class, designed for high-performance mathematical and scientific computing. It leverages the power of GPU acceleration via the Vulkan API to perform a vast array of parallel computations, from basic arithmetic to complex matrix operations, statistical analysis, and neural network functions.

_Note: This library relies on a custom Vulkan wrapper ([`vkcontext.h`](../include/vkcontext.h)) for managing the GPU context and assumes that compute shader binaries (`spirv_bin.h`) are available (auto-generated into the project output folder by CMake, using the provided CMakeLists.txt in this repository) or [precompiled](../include/spirv_bin_precompiled.h)._

---

## ✨ Features
- **N-Dimensional Data**: Natively create and manipulate tensors of any shape and dimension.
- **GPU Accelerated**: All mathematical operations are executed on the GPU via Vulkan compute shaders for maximum parallelism and speed.
- **Rich Mathematical Toolkit**: An extensive set of operations including linear algebra, statistical functions, random number generation, and element-wise calculations.
- **Neural Network Support**: Includes specialized functions for weight initialization, activation functions, and their derivatives.
- **Dynamic Manipulation**: Easily reshape, concatenate, pad, transpose, and perform complex transformations like convolution and pooling.
- **High-Level Abstraction**: Simplifies complex GPU operations into intuitive C++ method calls and operator overloads.

---

## 🛠️ Classes and Methods

### Constructors & Destructors ###
These methods handle the creation, copying, moving, and destruction of `NGrid` objects.

| **Method**| **Description**|
| :--- | :--- |
| `NGrid()` | Default constructor that initializes an empty, shapeless grid. |
| `NGrid(Args... args)` | Variadic template constructor to define the grid's shape, e.g., `NGrid(10, 20, 3)`. |
| `NGrid(const std::vector<uint32_t>& shape)` | Constructs a grid with the specified shape from a vector. |
| `NGrid(std::initializer_list<uint32_t> shape)`| Constructs a grid with the specified shape from an initializer list. |
| `NGrid(std::vector<float_t> source_vector)` | Constructs a 1D grid and fills it with data from a `std::vector`. |
| `NGrid(const float_t* source_array, ...)` | Constructs a 1D grid and fills it with data from a raw C-style array. |
| `NGrid(const NGrid& other)` | Copy constructor. Creates a deep copy of another grid's data on the GPU. |
| `NGrid(NGrid&& other) noexcept` | Move constructor. Efficiently transfers ownership of GPU resources. |
| `~NGrid()` | Destructor that cleans up and releases all associated GPU resources. |

---
### Assignment ###
Methods for assigning data to an existing `NGrid` instance.

| **Method**| **Description**|
| :--- | :--- |
| `operator=(const NGrid& other)` | Copy assignment. Replaces the grid's content with a copy of another grid's data. |
| `operator=(NGrid&& other) noexcept` | Move assignment. Transfers ownership of GPU resources from another grid. |
| `operator=(const std::vector<float_t>& data)` | Assigns data from a `std::vector` to the grid. Alias for `set()`. |
| `operator=(const float_t* data)` | Assigns data from a raw C-style array to the grid. Alias for `set()`. |

---
### Getters & Setters ###
Methods for retrieving information and data from the grid or setting specific values.

| **Method**| **Description**|
| :--- | :--- |
| `set(index, value)` | Sets the element at a specific multi-dimensional index to a given value. |
| `set(data, ...)` | Sets grid contents from a `std::vector`, C-style array, or another `NGrid`. |
| `get(flat_index)` | Retrieves a single `float_t` value from a flattened 1D index. |
| `get()` | Retrieves all grid elements as a `std::vector<float_t>`. |
| `get(read_elements, offset)` | Retrieves a specific slice of the grid data into a `std::vector<float_t>`. |
| `get_buffer()` | Returns a pointer to the underlying Vulkan buffer holding the grid data. |
| `get_shape_buffer()` | Returns a pointer to the Vulkan buffer holding the grid's shape information. |
| `get_dimensions()` | Returns the number of dimensions (rank) of the grid. |
| `get_size(dimension)` | Returns the size of a specific dimension. |
| `get_elements()` | Returns the total number of elements in the grid. |
| `get_shape()` | Returns the shape of the grid as a `std::vector<uint32_t>`. |
| `get_shapestring()` | Returns the shape of the grid as a formatted string (e.g., `"[10, 20, 3]"`). |
| `subgrid(offset, shape)` | Extracts a new `NGrid` view from a region of the current grid. |

---
### 🎲 Fill Operations ###
Quickly populate the entire grid with specific values or random distributions.

| **Method**| **Description**|
| :--- | :--- |
| `fill(value)` | Fills the entire grid with a specified floating-point value. |
| `fill_zero()` | Fills the grid with `0.0f`. |
| `fill_identity()` | Fills the grid with the identity matrix. |
| `fill_random_gaussian(mu, sigma)` | Fills with random numbers from a Gaussian (normal) distribution. |
| `fill_random_uniform(min, max)` | Fills with random numbers from a uniform distribution. |
| `fill_random_uniform_int(min, max)`| Fills with random integers from a uniform distribution. |
| `fill_random_binary(ratio)` | Fills with `0.0f` or `1.0f` based on a given ratio. |
| `fill_random_sign(ratio)` | Fills with `-1.0f` or `1.0f` based on a given ratio. |
| `fill_range(start, step)` | Fills the grid with a sequence of numbers. |
| `fill_dropout(ratio)` | Applies dropout by setting a random fraction of elements to zero. |
| `fill_index()` | Fills each element with its flattened 1D index. |

---
### 🧠 Neural Net Weight Initialization ###
Specialized methods to initialize weight matrices for neural networks, based on common techniques.

| **Method**| **Description**|
| :--- | :--- |
| `weightinit_tanh_normal(fan_in, fan_out)` | Glorot/Xavier normal initialization. |
| `weightinit_tanh_uniform(fan_in, fan_out)`| Glorot/Xavier uniform initialization. |
| `weightinit_sigmoid(fan_in, fan_out)` | Sigmoid-specific weight initialization. |
| `weightinit_relu(fan_in)` | He normal initialization for ReLU activation. |
| `weightinit_elu(fan_in)` | He normal initialization for ELU activation. |

---
### 📊 Distribution Properties ###
Calculates statistical properties of the grid's data, reducing the entire grid to a single value.

| **Method**| **Description**|
| :--- | :--- |
| `min()` | Returns the minimum value in the grid. |
| `max()` | Returns the maximum value in the grid. |
| `maxabs()` | Returns the maximum absolute value in the grid. |
| `mean()` | Returns the arithmetic mean of all elements. |
| `median()` | Returns the median value of all elements. |
| `var(sample_var)` | Returns the variance (sample or population). |
| `stdev()` | Returns the standard deviation of all elements. |
| `kurt()` | Returns the kurtosis of the distribution. |
| `skew()` | Returns the skewness of the distribution. |
| `sum()` | Returns the sum of all elements. |
| `product()` | Returns the product of all elements. |

---
### Arithmetic Operations ###
Element-wise and scalar arithmetic using convenient operator overloads.

| **Method**| **Description**|
| :--- | :--- |
| `operator+(value)` / `operator-(value)` | Adds/subtracts a scalar value to/from each element. |
| `operator+(other)` / `operator-(other)` | Performs element-wise addition/subtraction with another grid. |
| `operator++()` / `operator--()` | Prefix increment/decrement. |
| `operator++(int)` / `operator--(int)` | Postfix increment/decrement. |
| `operator+=(value)` / `operator-=(value)` | In-place scalar addition/subtraction. |
| `operator+=(other)` / `operator-=(other)` | In-place element-wise addition/subtraction. |
| `operator*(factor)` / `operator/(quotient)`| Multiplies/divides each element by a scalar. |
| `operator*=(factor)` / `operator/=(quotient)`| In-place scalar multiplication/division. |
| `operator%(num)` / `operator%=(value)` | Computes element-wise or in-place modulo with a scalar. |

---
### Matrix & Vector Operations ###
Handles dot products, matrix multiplications, and element-wise products.

| **Method**| **Description**|
| :--- | :--- |
| `operator*(other)` | Alias for `matrix_product`. |
| `operator*=(other)` | In-place matrix product. Modifies the current grid. |
| `scalar_product(other)` | Computes the dot/scalar product with another grid. Returns a `float_t`. |
| `matrix_product(other)` | Computes the matrix product (matmul). |
| `Hadamard_product(other)` | Computes the element-wise (Hadamard) product. |
| `Hadamard_division(other)` | Computes the element-wise (Hadamard) division. |
| `operator/(other)` | Alias for matrix product with the inverse of `other`. |

---
### Mathematical Functions ###
Applies common mathematical functions to each element of the grid.

| **Method**| **Description**|
| :--- | :--- |
| `pow(exponent)` / `operator^(exponent)` | Raises each element to a scalar power. |
| `pow(other)` / `operator^(other)` | Performs element-wise exponentiation with another grid as the exponent. |
| `sqrt()` | Computes the square root of each element. |
| `log(base)` | Computes the logarithm of each element for a given base. |
| `exp()` | Computes `e` raised to the power of each element. |
| `round()` / `floor()` / `ceil()` | Rounds, floors, or ceils each element. |
| `abs()` | Computes the absolute value of each element. |
| `sign()` | Returns the sign of each element (`-1`, `0`, or `1`). |
| `min(value)` / `max(value)` | Computes element-wise min/max against a scalar value. |
| `min(other)` / `max(other)` | Computes element-wise min/max against another grid. |

---
### 📐 Trigonometric Functions ###
Applies trigonometric and hyperbolic functions to each element.

| **Method**| **Description**|
| :--- | :--- |
| `cos(unit)` / `sin(unit)` / `tan(unit)` | Computes cosine, sine, or tangent. Input can be specified as `RAD`, `DEG`, `HOURS12`/`HOURS24`(12h/24h time angle), `GON`(geodetic, full circle = 400), `PERCENT` (full circle = 100) or `NORMAL` (full circle = 1.0)). |
| `acos(unit)`/`asin(unit)`/`atan(unit)` | Computes arc-cosine, arc-sine, or arc-tangent. Output unit can be specified. |
| `cosh()` / `sinh()` / `tanh()` | Computes hyperbolic cosine, sine, or tangent. |
| `acosh()` / `asinh()` / `atanh()` | Computes inverse hyperbolic cosine, sine, or tangent. |

---
### Activation Functions & Derivatives ###
A suite of common neural network activation functions and their corresponding derivatives. The `ActFunc` enum (`RELU`, `LRELU`, `ELU`, `LELU`, `SIGMOID`, `TANH`, `IDENT`) can be used with the generic methods.

| **Method**| **Description**|
| :--- | :--- |
| `activation(ActFunc)` | Applies a specified activation function element-wise. |
| `derivative(ActFunc)` | Computes the derivative of a specified activation function element-wise. |
| `ident()` / `ident_drv()` | Identity function (`f(x) = x`) and its derivative (`1`). |
| `sigmoid()` / `sigmoid_drv()` | Sigmoid activation and its derivative. |
| `relu(alpha)` / `relu_drv(alpha)` | (Leaky) Rectified Linear Unit and its derivative. |
| `elu(alpha)` / `elu_drv(alpha)` | (Leaky) Exponential Linear Unit and its derivative. |
| `tanh()` / `tanh_drv()` | Hyperbolic tangent activation and its derivative. |

---
### Dynamic Handling & Conversion ###
Methods for transforming the grid's shape, structure, and content.

| **Method**| **Description**|
| :--- | :--- |
| `flatten()` | Reshapes the grid into a 1D vector. |
| `reshape(new_shape, ...)` | Changes the grid's shape whilst preserving elements which overlap with the previous shape. Can also be used for resizing. |
| `concatenate(other, axis)` | Joins another grid along a specified axis. |
| `padding(amount, value)` | Adds padding of a certain value around the grid. |
| `transpose(target_axis_order)` | Reorders the dimensions of the grid. |
| `convolution(kernel, ...)` | Performs a 2D convolution with a given kernel. |
| `pool_max(window, stride)` | Performs max pooling over a window. |
| `pool_mean(window, stride)` | Performs average pooling over a window. |
| `sort(ascending)` | Sorts the elements of a 1D grid in ascending or descending order. |
| `lu_decomp(L, U, P)` | Performs LU decomposition with partial pivoting. |
| `inverse()` | Computes the inverse of a square matrix (or pseudo-inverse in case of non-square 2d grids). |
| `mirror(axes)` | Flips the grid along the specified axes. |

---
### Elementwise Comparison & Logical Operations ###
These operators return a new `NGrid` where each element is `1.0f` if the condition is true and `0.0f` if false.

| **Method**| **Description**|
| :--- | :--- |
| `operator>(value/other)` | Element-wise greater than. |
| `operator>=(value/other)` | Element-wise greater than or equal to. |
| `operator==(value/other)` | Element-wise equal to. |
| `operator!=(value/other)` | Element-wise not equal to. |
| `operator<(value/other)` | Element-wise less than. |
| `operator<=(value/other)` | Element-wise less than or equal to. |
| `operator&&(value/other)` | Element-wise logical AND. |
| `operator||(value/other)` | Element-wise logical OR. |
| `operator!()` | Element-wise logical NOT. |

---
### Miscellaneous ###
Utility and configuration methods.

| **Method**| **Description**|
| :--- | :--- |
| `print(...)` | Prints a formatted representation of the grid to the console. |
| `set_workgroup_size_1d(size)` | Sets the default Vulkan workgroup size for 1D dispatches. |
| `set_workgroup_size_2d(size)` | Sets the default Vulkan workgroup size (x & y) for 2D dispatches. |
| `set_fence_timeout_nanosec(timeout)`| Sets the GPU synchronization fence timeout in nanoseconds. |