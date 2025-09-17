# CGrid Library (`cgrid.h`)

## 📜 Summary

The `cgrid.h` library introduces the `CGrid` class, an extension of `NGrid` designed specifically for n-dimensional arrays of complex numbers. By leveraging two `NGrid` objects internally—one for the real parts and one for the imaginary parts—`CGrid` inherits the GPU-accelerated performance of `NGrid` while providing a comprehensive suite of operations tailored for complex arithmetic, linear algebra, and data manipulation.

_Note: This library relies on a custom Vulkan wrapper ([`vkcontext.h`](../include/vkcontext.h)) for managing the GPU context and assumes that compute shader binaries (`spirv_bin.h`) are available (auto-generated into the project output folder by CMake, using the provided CMakeLists.txt in this repository) or [precompiled](../include/spirv_bin_precompiled.h)._

---

## ✨ Features

- **Complex N-Dimensional Data**: Natively handles tensors of complex numbers of any shape and dimension.
- **GPU Accelerated**: All complex number operations are performed on the GPU by using the underlying `NGrid` Vulkan compute shaders.
- **Full Complex Number Support**: Provides overloads for constructors, assignments, and operations to seamlessly handle complex values and `std::complex<float_t>` types.
- **Extensive Mathematical Toolkit**: Includes complex-aware implementations of arithmetic, exponentiation, logarithms, and advanced matrix operations.
- **Seamless Integration with NGrid**: Allows for operations between `CGrid` and `NGrid` objects, facilitating mixed-type computations.
- **Advanced Matrix Operations**: Features complex-number versions of matrix manipulation and decomposition.

---

### 🛠️ Constructors & Destructors
Methods for creating and destroying `CGrid` instances.

| **Method**| **Description**|
| :--- | :--- |
| `CGrid()` | The default constructor initializes an empty array. |
| `CGrid(...)` | Parametric default constructor for multi-dimensional array, overload for variadic template. |
| `CGrid(const std::vector<uint32_t>& shape)` | Parametric default constructor for multi-dimensional array, overload for `std::vector`. |
| `CGrid(std::initializer_list<uint32_t> shape)`| Parametric default constructor for multi-dimensional array, overload for `std::initializer_list`. |
| `CGrid(const std::vector<float_t>& source_vector)` | Constructs a 1D array from a vector of real numbers. |
| `CGrid(const std::vector<std::complex<float_t>>& source_vector)` | Constructs a 1D array from a vector of complex numbers. |
| `CGrid(const NGrid& other)`| Constructs a new `CGrid` from an existing `NGrid` (real part only). |
| `CGrid(const CGrid& other)`| Copy constructor. |
| `CGrid(NGrid&& other) noexcept`| Move constructor for an `NGrid` instance. |
| `CGrid(CGrid&& other) noexcept`| Move constructor. |
| `~CGrid()` | Destructor. |

---

### 🔀 Assignment
Methods for assigning values to a `CGrid` instance.

| **Method**| **Description**|
| :--- | :--- |
| `operator=` | Copy assignment for `CGrid` and `NGrid` instances. |
| `operator=(CGrid&& other) noexcept`| Move assignment for `CGrid` and `NGrid` instances. |
| `operator=` | Assigns the contents of a `std::vector<float_t>` or a `std::vector<std::complex<float_t>>` to the grid. |

---

### 📥 Getters & Setters
Methods for getting and setting elements or properties of the `CGrid` instance.

| **Method**| **Description**|
| :--- | :--- |
| `CGrid& set(...)` | Destructive setter methods that modify the `CGrid` instance itself, then return a reference to `this`. Overloads are provided to set a single complex value by index, or to copy data from a `std::vector` of complex or real numbers, or another `CGrid` or `NGrid` instance. |
| `std::complex<float_t> get(...)` | Non-destructive getter methods that return a copy of the requested data. Overloads are provided to retrieve a single complex value by index, a subset of elements, or the entire underlying data as a `std::vector<std::complex<float_t>>`. |
| `get_dimensions()` | Returns the number of dimensions of the array. |
| `get_size(uint32_t dimension)` | Returns the size of the specified dimension. |
| `get_elements()` | Returns the total number of elements in the array. |
| `get_shape()` | Returns the shape of the array as a `const std::vector<uint32_t>&`. |
| `rows()` | Returns the number of rows (size of the first dimension). |
| `cols()` | Returns the number of columns (size of the second dimension). |
| `get_shapestring()` | Returns the shape of the array as a formatted string, e.g., `{2,3,4}`. |
| `subgrid(const std::vector<uint32_t>& source_offset, const std::vector<uint32_t>& subgrid_shape)`| Slices a subarray out of the parent array. |

---

### 🔄 Fill
Destructive methods that modify the `CGrid` instance by filling it with specific values.

| **Method**| **Description**|
| :--- | :--- |
| `fill(const std::complex<float_t> complex_value)`| Fills the entire grid with a single `complex<float_t>` value. |
| `fill(const float_t value)`| Fills the entire grid with a single `float_t` value. |
| `fill_zero()` | Fills the grid with zeros (`0 + 0i`). |

---

### ➕ Addition
Methods for addition operations.

| **Method**| **Description**|
| :--- | :--- |
| `sum()` | Returns the sum of all complex array elements. |
| `operator+(const float_t value)`| Returns a new `CGrid` with `value` added to each element. |
| `operator+(const std::complex<float_t> complex_value)`| Returns a new `CGrid` with `complex_value` added to each element. |
| `operator+(const CGrid& other)`| Returns a new `CGrid` that is the element-wise sum of this grid and another. |
| `operator+(const NGrid& other)`| Returns a new `CGrid` that is the element-wise sum of this grid and a real `NGrid`. |
| `operator++()` | Prefix increment. |
| `operator++(int)` | Postfix increment. |
| `operator+=(const float_t value)` | Adds `value` to each element in-place. |
| `operator+=(const std::complex<float_t> complex_value)` | Adds `complex_value` to each element in-place. |
| `operator+=(const NGrid& other)` | Performs an element-wise addition with a real `NGrid` in-place. |
| `operator+=(const CGrid& other)` | Performs an element-wise addition with `other` in-place. |

---

### ➖ Subtraction
Methods for subtraction operations.

| **Method**| **Description**|
| :--- | :--- |
| `operator-(const float_t value)`| Returns a new `CGrid` with `value` subtracted from each element. |
| `operator-(const std::complex<float_t> complex_value)`| Returns a new `CGrid` with `complex_value` subtracted from each element. |
| `operator-(const CGrid& other)`| Returns a new `CGrid` that is the element-wise difference of this grid and another. |
| `operator-(const NGrid& other)`| Returns a new `CGrid` that is the element-wise difference of this grid and a real `NGrid`. |
| `operator--()` | Prefix decrement. |
| `operator--(int)` | Postfix decrement. |
| `operator-=(const float_t value)` | Subtracts `value` from each element in-place. |
| `operator-=(const std::complex<float_t> complex_value)` | Subtracts `complex_value` from each element in-place. |
| `operator-=(const CGrid& other)` | Performs an element-wise subtraction with `other` in-place. |
| `operator-=(const NGrid& other)` | Performs an element-wise subtraction with a real `NGrid` in-place. |

---

### ✖️ Multiplication
Methods for multiplication operations.

| **Method**| **Description**|
| :--- | :--- |
| `product()` | Returns the product of all array elements. |
| `operator*(const float_t factor)`| Returns a new `CGrid` with each element multiplied by `factor`. |
| `operator*(const std::complex<float_t> complex_factor)`| Returns a new `CGrid` with each element multiplied by `complex_factor`. |
| `operator*=(const float_t factor)`| Multiplies each element by `factor` in-place. |
| `operator*=(const std::complex<float_t> complex_factor)`| Multiplies each element by `complex_factor` in-place. |
| `operator*(const CGrid& other)`| Alias for `matrix_product()`. |
| `operator*(const NGrid& other)`| Alias for `matrix_product()`. |
| `operator*=(const NGrid& other)`| Performs a matrix product with `other` in-place. |
| `operator*=(const CGrid& other)`| Performs a matrix product with `other` in-place. |
| `scalar_product(const CGrid& other)`| Returns the scalar (dot) product of this grid and another. |
| `scalar_product(const NGrid& other)`| Returns the scalar (dot) product of this grid and another. |
| `matrix_product(const CGrid& other)`| Returns a new `CGrid` that is the matrix product of this grid and another. |
| `matrix_product(const NGrid& other)`| Returns a new `CGrid` that is the matrix product of this grid and a real `NGrid`. |
| `Hadamard_product(const CGrid& other)`| Returns a new `CGrid` that is the element-wise product of this grid and another. |
| `Hadamard_product(const NGrid& other)`| Returns a new `CGrid` that is the element-wise product of this grid and a real `NGrid`. |

---

### ➗ Division
Methods for division operations.

| **Method**| **Description**|
| :--- | :--- |
| `operator/(const float_t divisor)`| Returns a new `CGrid` with each element divided by `divisor`. |
| `operator/(const std::complex<float_t> divisor)`| Returns a new `CGrid` with each element divided by `divisor`. |
| `operator/=(const float_t divisor)`| Divides each element by `divisor` in-place. |
| `operator/=(const std::complex<float_t> divisor)`| Divides each element by `divisor` in-place. |
| `Hadamard_division(const CGrid& other)`| Performs element-wise division with another `CGrid`. |
| `Hadamard_division(const NGrid& other)`| Performs element-wise division with a real `NGrid`. |
| `operator/(const NGrid& other)`| Returns a new `CGrid` that is the matrix product with the inverse of a real `NGrid`. |
| `operator/(const CGrid& other)`| Returns a new `CGrid` that is the matrix product with the inverse of `other`. |

---

### 📈 Exponentiation & Logarithm
Methods for exponentiation and logarithm operations.

| **Method**| **Description**|
| :--- | :--- |
| `pow(const float_t exponent)` | Returns a new `CGrid` with each element raised to the power of `exponent`. |
| `pow(const std::complex<float_t> exponent)` | Returns a new `CGrid` with each element raised to the power of `exponent`. |
| `operator^(const float_t exponent)` | Alias for `pow(exponent)`. |
| `operator^(const std::complex<float_t> exponent)` | Alias for `pow(exponent)`. |
| `operator^=(const float_t exponent)`| Performs `pow(exponent)` in-place. |
| `operator^=(const std::complex<float_t> exponent)`| Performs `pow(exponent)` in-place. |
| `pow(const NGrid& other)` | Returns a new `CGrid` with each element raised to the power of the corresponding element in `other`. |
| `pow(const CGrid& other)` | Returns a new `CGrid` with each element raised to the power of the corresponding element in `other`. |
| `operator^(const NGrid& other)`| Alias for `pow(other)`. |
| `operator^(const CGrid& other)`| Alias for `pow(other)`. |
| `sqrt()`| Returns a new `CGrid` with the square root of each element. |
| `log(const float_t base)` | Returns a new `CGrid` with the logarithm of each element. Default base is `e` (2.718...). |
| `log(const std::complex<float_t> base)` | Returns a new `CGrid` with the logarithm of each element to a complex base. |
| `exp()` | Returns a new `CGrid` with the exponent of each element. |

---

### 📐 Complex Number Properties
Methods to get properties of complex numbers.

| **Method**| **Description**|
| :--- | :--- |
| `conjugate()`| Returns a new `CGrid` with the conjugate of each element. |
| `abs()` | Returns a new `NGrid` with the absolute value (magnitude) of each element. |
| `arg()`| Returns a new `NGrid` with the argument (angle) of each element. |
| `norm()`| Returns a new `NGrid` with the squared magnitude of each element. |

---

### 🧠 Advanced Matrix Operations
Methods for complex data transformations.

| **Method**| **Description**|
| :--- | :--- |
| `flatten()` | Returns a new `CGrid` with all dimensions collapsed into a single dimension. |
| `reshape(...)` | Returns a new `CGrid` with the specified new shape. Overloads exist for `std::vector`, `std::initializer_list`, and variadic templates. |
| `concatenate(...)` | Concatenates this `CGrid` with another along the specified `axis`. |
| `padding(amount, value)` | Adds padding of a complex value around the grid. |
| `transpose(target_axis_order)` | Reorders the dimensions of the grid. |
| `convolution(kernel, ...)` | Performs a 2D convolution with a real or complex kernel and optional padding. |
| `lu()` | Performs LU decomposition with partial pivoting on the complex matrix. |
| `inverse()` | Computes the inverse of a square complex matrix. |
| `is_invertible()` | Checks if the complex matrix is invertible. |
| `mirror(axes)` | Flips the grid along the specified axes. |
| `remap(index_map)` | Reassigns each element to a new position based on an `NGrid` index map. |

---

### 🎁 Miscellaneous
Utility and configuration methods.

| **Method**| **Description**|
| :--- | :--- |
| `operator NGrid()` | Explicit conversion operator to `NGrid`. This will copy the real part of the `CGrid` into a new `NGrid`. |
| `print(...)` | Prints a formatted representation of the grid to the console. |
| `flat_index(...)` | Returns the 1D flat index from a multidimensional index. |