# CGrid Library (`cgrid.h`)

## 📜 Summary

The `cgrid.h` library introduces the `CGrid` class, an extension of `NGrid` designed specifically for n-dimensional arrays of complex numbers. By leveraging two `NGrid` objects internally—one for the real parts and one for the imaginary parts—`CGrid` inherits the GPU-accelerated performance of `NGrid` while providing a comprehensive suite of operations tailored for complex arithmetic, linear algebra, and data manipulation.

*Note: This library relies on [`ngrid.h`](../include/ngrid.h) and the underlying Vulkan wrapper ([`vkcontext.h`](../include/vkcontext.h)). It assumes that all necessary compute shaders and binaries are in place.*

-----

## ✨ Features

  - **Complex N-Dimensional Data**: Natively handles tensors of complex numbers of any shape and dimension.
  - **GPU Accelerated**: All complex number operations are performed on the GPU by using the underlying `NGrid` Vulkan compute shaders.
  - **Full Complex Number Support**: Provides overloads for constructors, assignments, and operations to seamlessly handle complex values and `std::complex<float_t>` types.
  - **Extensive Mathematical Toolkit**: Includes complex-aware implementations of arithmetic, exponentiation, logarithms, and advanced matrix operations.
  - **Seamless Integration with NGrid**: Allows for operations between `CGrid` and `NGrid` objects, facilitating mixed-type computations.
  - **Advanced Matrix Operations**: Features complex-number versions of matrix multiplication, inverse, LU decomposition, and other linear algebra functions.

-----

## Classes and Methods

### 🧱 Constructors & Destructors

These methods handle the creation, copying, moving, and destruction of `CGrid` objects, with support for complex number initialization.

| **Method**| **Description**|
| :--- | :--- |
| `CGrid()` | Default constructor that initializes an empty, shapeless complex grid. |
| `CGrid(Args... args)` | Variadic template constructor to define the grid's shape, e.g., `CGrid(10, 20, 3)`. |
| `CGrid(const std::vector<uint32_t>& shape)` | Constructs a complex grid with the specified shape from a vector. |
| `CGrid(std::initializer_list<uint32_t> shape)`| Constructs a complex grid with the specified shape from an initializer list. |
| `CGrid(std::vector<float_t> source_vector)` | Constructs a 1D complex grid with imaginary parts initialized to zero, and real parts filled with data from a `std::vector`. |
| `CGrid(NGrid&& other) noexcept` | Move constructor. Efficiently transfers ownership of GPU resources from a `NGrid` object, initializing `CGrid::real` with `other` and `CGrid::imag` with an empty `NGrid`. |
| `CGrid(CGrid&& other) noexcept` | Move constructor. Efficiently transfers ownership of both real and imaginary parts from another `CGrid`. |
| `CGrid(const NGrid& other)` | Copy constructor. Creates a deep copy of an `NGrid`'s data into the real part of the `CGrid`. The imaginary part is initialized to zero. |
| `CGrid(const CGrid& other)` | Copy constructor. Creates a deep copy of another complex grid's data on the GPU. |
| `~CGrid()` | Destructor that cleans up and releases all associated GPU resources. |

-----

### 🟰 Assignment

Methods for assigning data to an existing `CGrid` instance, including from complex `std::vector`s.

| **Method**| **Description**|
| :--- | :--- |
| `operator=(const NGrid& other)` | Copy assignment. Replaces the real part of the grid's content with a copy of an `NGrid`'s data. The imaginary part is set to zero. |
| `operator=(const CGrid& other)` | Copy assignment. Replaces the grid's content with a deep copy of another complex grid's data. |
| `operator=(NGrid&& other) noexcept` | Move assignment. Transfers ownership of GPU resources from a `NGrid` object to the real part of the `CGrid`. The imaginary part is set to an empty `NGrid`. |
| `operator=(CGrid&& other) noexcept` | Move assignment. Transfers ownership of both real and imaginary parts from another `CGrid`. |
| `operator=(const std::vector<float_t>& data)` | Assigns data from a `std::vector` to the real part of the grid. |
| `operator=(const std::vector<std::complex<float_t>>& data)`| Assigns data from a `std::vector` of complex numbers to the grid. |
| `operator=(const float_t* data)` | Assigns data from a raw C-style array to the real part of the grid. |

-----

### ✔️ Getters & Setters

Methods for retrieving and setting complex data and properties of the grid.

| **Method**| **Description**|
| :--- | :--- |
| `set(index, value)` | Sets the element at a specific multi-dimensional index to a real `float_t` value or a `std::complex<float_t>` value. |
| `set(data, ...)` | Sets grid contents from a `std::vector` of `float_t`s or `std::complex<float_t>`s, a C-style array, or another `NGrid` or `CGrid`. |
| `get(flat_index)` | Retrieves a single `std::complex<float_t>` value from a flattened 1D index. |
| `get()` | Retrieves all grid elements as a `std::vector<std::complex<float_t>>`. |
| `get(read_elements, offset)` | Retrieves a specific slice of the complex grid data into a `std::vector<std::complex<float_t>>`. |
| `get_dimensions()` | Returns the number of dimensions (rank) of the grid. |
| `get_size(dimension)` | Returns the size of a specific dimension. |
| `get_elements()` | Returns the total number of elements in the grid. |
| `get_shape()` | Returns the shape of the grid as a `std::vector<uint32_t>`. |
| `get_shapestring()` | Returns the shape of the grid as a formatted string (e.g., `"[10, 20, 3]"`). |
| `subgrid(offset, shape)` | Extracts a new `CGrid` view from a region of the current grid. |

-----

### 🎲 Fill Operations

Quickly populate the entire complex grid with specific values.

| **Method**| **Description**|
| :--- | :--- |
| `fill(value)` | Fills the entire grid with a specified `float_t` value (imaginary part is set to zero). |
| `fill(complex_value)` | Fills the entire grid with a specified `std::complex<float_t>` value. |
| `fill_zero()` | Fills the grid with `0.0f + 0.0fi`. |

-----

### ➕ Arithmetic Operations

Element-wise and scalar arithmetic using convenient operator overloads, adapted for complex numbers.

| **Method**| **Description**|
| :--- | :--- |
| `sum()` | Returns the sum of all elements as a `std::complex<float_t>`. |
| `operator+(value/other)` / `operator-(value/other)` | Adds/subtracts a real scalar, a complex scalar, an `NGrid`, or another `CGrid`. |
| `operator++()` / `operator--()` | Prefix increment/decrement. Adds/subtracts `1 + 0i`. |
| `operator++(int)` / `operator--(int)` | Postfix increment/decrement. |
| `operator+=(value/other)` / `operator-=(value/other)`| In-place addition/subtraction with a real scalar, a complex scalar, an `NGrid`, or a `CGrid`. |
| `product()` | Returns the product of all elements as a `std::complex<float_t>`. |
| `operator*(factor)` / `operator/(divisor)`| Multiplies/divides each element by a real or complex scalar. |
| `operator*=(factor)` / `operator/=(divisor)`| In-place scalar multiplication/division. |

-----

### 🔢 Matrix & Vector Operations

Handles dot products, matrix multiplications, and element-wise products for complex matrices.

| **Method**| **Description**|
| :--- | :--- |
| `operator*(other)` | Alias for `matrix_product` when multiplying by an `NGrid` or another `CGrid`. |
| `operator*=(other)` | In-place matrix product with an `NGrid` or another `CGrid`. |
| `scalar_product(other)` | Computes the dot/scalar product with an `NGrid` or another `CGrid`. Returns a `std::complex<float_t>`. |
| `matrix_product(other)` | Computes the matrix product (matmul) with an `NGrid` or another `CGrid`. |
| `Hadamard_product(other)` | Computes the element-wise (Hadamard) product with an `NGrid` or another `CGrid`. |
| `Hadamard_division(other)` | Computes the element-wise (Hadamard) division with an `NGrid` or another `CGrid`. |
| `operator/(other)` | Alias for matrix product with the inverse of `other`. Works with `NGrid` or `CGrid`. |

-----

### 🧮 Mathematical Functions

Applies common mathematical functions to each element of the complex grid.

| **Method**| **Description**|
| :--- | :--- |
| `pow(exponent)` / `operator^(exponent)` | Raises each element to a real or complex scalar power. |
| `pow(other)` / `operator^(other)` | Performs element-wise exponentiation with a `NGrid` or `CGrid` as the exponent. |
| `sqrt()` | Computes the square root of each element. |
| `log(base)` | Computes the logarithm of each element for a given real or complex base. Default is base `e`. |
| `exp()` | Computes `e` raised to the power of each element. |
| `magnitude()` / `abs()` | Returns a new `NGrid` containing the magnitude (absolute value) of each complex element. |

-----

### 🛠️ Advance Matrix Operations

Methods for transforming the complex grid's shape, structure, and content.

| **Method**| **Description**|
| :--- | :--- |
| `flatten()` | Reshapes the complex grid into a 1D vector. |
| `reshape(new_shape, ...)` | Changes the complex grid's shape, preserving elements. Can be used for resizing and initialized with a real or complex default value. |
| `concatenate(other, axis)` | Joins another `NGrid` or `CGrid` along a specified axis. |
| `padding(amount, value)` | Adds padding of a real or complex value around the grid. |
| `transpose(target_axis_order)` | Reorders the dimensions of the grid. |
| `convolution(kernel, ...)` | Performs a 2D convolution with a real or complex kernel and optional padding. |
| `lu()` | Performs LU decomposition with partial pivoting on the complex matrix. Returns the L, U, P matrices and the swap count as a custom struct. |
| `inverse()` | Computes the inverse of a square complex matrix (or pseudo-inverse in case of non-square 2d grids). |
| `is_invertible()` | Checks if the complex matrix is invertible. |
| `is_invertible(U)` | Checks if a matrix which has the corresponding provided Upper Triangular matrix U is invertible. |
| `mirror(axes)` | Flips the grid along the specified axes. |
| `remap(index_map)` | Reassigns each element to a new position based on an `NGrid` index map. |

-----

### 🎁 Miscellaneous

Utility and configuration methods.

| **Method**| **Description**|
| :--- | :--- |
| `operator NGrid()` | Explicit conversion operator to `NGrid`. This will copy the real part of the `CGrid` into a new `NGrid`. |
| `set_workgroup_size_1d(size)` | Sets the default Vulkan workgroup size for 1D dispatches. |
| `set_workgroup_size_2d(size)` | Sets the default Vulkan workgroup size (x & y) for 2D dispatches. |
| `set_fence_timeout_nanosec(timeout)`| Sets the GPU synchronization fence timeout in nanoseconds. |
| `print(...)` | Prints a formatted representation of the complex grid to the console. |