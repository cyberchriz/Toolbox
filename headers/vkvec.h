// author and copyright: Christian Suer (cyberchriz)
// description: library for parallel floating point data structure computations on the GPU (using Vulkan)

#ifndef VKVEC_H
#define VKVEC_H

#include "angular.h"
#include "log.h"
#include "vkcontext.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <float.h>
#include <iostream>
#include <math.h>
#include <ostream>
#include <seed.h>
#include <string>
#include <time.h>
#include <type_traits>
#include <utility>
#include <vector>
#include <vulkan_core.h>
#include <timelog.h>

// default constants (change if needed)
constexpr uint32_t DEFAULT_DEVICE_ID = 0;
constexpr char* APPLICATION_NAME = "VkVec";
constexpr uint32_t APPLICATION_MAJOR_VERSION = 1;
constexpr uint32_t APPLICATION_MINOR_VERSION = 0;
constexpr uint32_t APPLICATION_PATCH_VERSION = 0;
constexpr uint32_t max_sets_within_pool = 10;
constexpr uint32_t compute_timeout_microsec = 100000; // set as 0 for compute without fences

// list of available activation functions
enum ActFunc {
    RELU,       // rectified linear unit (ReLU)
    LRELU,      // leaky rectified linear unit (LReLU)
    ELU,        // exponential linar unit (ELU)
    LELU,       // leaky exponential linear unit
    SIGMOID,    // sigmoid (=logistic)
    TANH,       // hyperbolic tangent (tanh), with angular unit radians
    IDENT       // identity function
};

// data structure class for parallel computing with Vulkan
class VkVec {
public:
    // +=================================+   
    // | Constructors & Destructors      |
    // +=================================+

    VkVec(const uint32_t rows = 1, const uint32_t cols = 1, const uint32_t depth = 1); // = parametric default constructor
    VkVec(VkVec&& other) noexcept; // = move constructor
    VkVec(const VkVec& other); // = copy constructor
    ~VkVec(); // destructor

    // +=================================+   
    // | getters & setters               |
    // +=================================+

    void set(const float_t value, const uint32_t row, const uint32_t col = 0, const uint32_t depth = 0);
    void set(std::vector<float_t>& data);
    float_t get(const uint32_t row, const uint32_t col = 0, const uint32_t depth = 0) const;
    std::vector<float> get() const;
    uint32_t get_dimensions() const;
    uint32_t get_rows() const;
    uint32_t get_cols() const;
    uint32_t get_depth() const;
    uint32_t get_elements() const;
    std::string get_shapestring() const;
    VkVec get_row(int32_t row_index) const;
    VkVec get_col(int32_t col_index) const;
    VkVec get_layer(int32_t layer_index) const;

    // +=================================+   
    // | Fill, Initialize                |
    // +=================================+

    void fill(const float_t value);
    void fill_zero();
    void fill_identity();
    void fill_random_gaussian(const float_t mu = 0.0f, const float_t sigma = 1.0f);
    void fill_random_uniform(const float_t min = 0.0f, const float_t max = 1.0f);
    void fill_random_uniform_int(const int32_t min = 0, const int32_t max = 9);
    void fill_random_binary(float_t ratio = 0.5f);
    void fill_random_sign(float_t ratio = 0.5f);
    void fill_range(const float_t start = 0.0f, const float_t step = 1.0f);
    void fill_dropout(float_t ratio = 0.2f);
    void fill_Xavier_normal(uint32_t fan_in, uint32_t fan_out);
    void fill_Xavier_uniform(uint32_t fan_in, uint32_t fan_out);
    void fill_Xavier_sigmoid(uint32_t fan_in, uint32_t fan_out);
    void fill_He_ReLU(uint32_t fan_in);
    void fill_He_ELU(uint32_t fan_in);

    // +=================================+   
    // | Distribution Properties         |
    // +=================================+

    float_t min() const;
    float_t max() const;
    float_t maxabs() const;
    float_t mean() const;
    float_t median() const;
    float_t variance() const;
    float_t stddev() const;
    float_t kurtosis() const;
    float_t skewness() const;

    // +=================================+   
    // | Addition                        |
    // +=================================+

    // returns the sum of all array elements
    float_t sum() const;
    VkVec operator+(const float_t value) const;
    VkVec operator+(const VkVec& other) const;
    VkVec& operator++(); // prefix increment
    VkVec operator++(int); // postfix increment
    void operator+=(const float_t value);
    void operator+=(const VkVec& other);

    // +=================================+   
    // | Substraction                    |
    // +=================================+

    VkVec operator-(const float_t value) const;
    VkVec operator-(const VkVec& other) const;
    VkVec& operator--(); // prefix decrement
    VkVec operator--(int); // postfix decrement
    void operator-=(const float_t value);
    void operator-=(const VkVec& other);

    // +=================================+   
    // | Multiplication                  |
    // +=================================+

    float_t product() const;
    VkVec operator*(const float_t factor) const;
    void operator*=(const float_t factor);
    VkVec operator*(const VkVec& other) const; // alias for matrix product
    void operator*=(const VkVec& other); // "equals matrix product"
    float_t scalar_product(const VkVec& other) const;
    VkVec matrix_product(const VkVec& other) const;
    VkVec Hadamard_product(const VkVec& other) const;

    // +=================================+   
    // | Division                        |
    // +=================================+

    VkVec operator/(const float_t quotient) const;
    void operator/=(const float_t quotient);
    VkVec Hadamard_division(const VkVec& other);

    // +=================================+   
    // | Modulo                          |
    // +=================================+

    void operator%=(const float_t value);
    VkVec operator%(const float_t num) const;

    // +=================================+   
    // | Exponentiation & Logarithm      |
    // +=================================+

    VkVec pow(const float_t exponent = 2.0f) const;
    VkVec operator^(const float_t exponent) const;
    void operator^=(const float_t exponent);
    VkVec pow(const VkVec& other) const;
    VkVec operator^(const VkVec& other) const;
    VkVec sqrt() const;
    VkVec log(float_t base = 2.718282) const;
    VkVec exp() const;

    // +=================================+   
    // | Rounding                        |
    // +=================================+

    VkVec round() const;
    VkVec floor() const;
    VkVec ceil() const;
    VkVec abs() const;

    // +=================================+   
    // | Min, Max                        |
    // +=================================+

    VkVec min(const float_t value) const;
    VkVec max(const float_t value) const;
    VkVec min(const VkVec& other) const;
    VkVec max(const VkVec& other) const;

    // +=================================+   
    // | Trigonometric Functions         |
    // +=================================+

    VkVec cos(AngularMeasure unit = RAD) const;
    VkVec sin(AngularMeasure unit = RAD) const;
    VkVec tan(AngularMeasure unit = RAD) const;
    VkVec acos(AngularMeasure unit = RAD) const;
    VkVec asin(AngularMeasure unit = RAD) const;
    VkVec atan(AngularMeasure unit = RAD) const;
    VkVec cosh(AngularMeasure unit = RAD) const;
    VkVec sinh(AngularMeasure unit = RAD) const;
    VkVec tanh(AngularMeasure unit = RAD) const;
    VkVec acosh(AngularMeasure unit = RAD) const;
    VkVec asinh(AngularMeasure unit = RAD) const;
    VkVec atanh(AngularMeasure unit = RAD) const;

    // +=================================+   
    // | Find, Replace                   |
    // +=================================+

    VkVec replace(const float_t& old_value, const float_t& new_value) const;
    VkVec replace_if(const VkVec& condition_map, const VkVec& replacing_map) const;
    VkVec replace_if(const VkVec& condition_map, const float_t replacing_value) const;
    uint32_t find(const float_t& value) const;
    VkVec sign() const;

    // +=================================+   
    // | Scaling                         |
    // +=================================+
    
    VkVec scale_minmax(float_t range_from = 0.0f, float_t range_to = 1.0f) const;
    VkVec scale_mean() const;
    VkVec scale_standardized() const;

    // +=================================+   
    // | Activation Functions            |
    // | (with Derivatives)              |
    // +=================================+

    VkVec activation(ActFunc activation_function) const;
    VkVec derivative(ActFunc activation_function) const;

    VkVec ident() const;
    VkVec ident_drv() const;
    VkVec sigmoid() const;
    VkVec sigmoid_drv() const;
    VkVec elu(float_t alpha = 0.01) const;
    VkVec elu_drv(float_t alpha = 0.01) const;
    VkVec relu(float_t alpha = 0.01) const;
    VkVec relu_drv(float_t alpha = 0.01) const;
    VkVec tanh_drv(AngularMeasure unit = RAD) const;

    // +=================================+   
    // | Outlier Treatment               |
    // +=================================+
    
    VkVec outliers_truncate(float_t z_score = 3.0f) const;
    VkVec outliers_mean_imputation(float_t z_score = 3.0f) const;
    VkVec outliers_value_imputation(float_t value = 0, float_t z_score = 3.0f) const;
    VkVec recover() const;

    // +=================================+   
    // | Assignment                      |
    // +=================================+

    VkVec& operator=(const VkVec& other); // copy assignment
    VkVec& operator=(VkVec&& other) noexcept; // move assignment

    // +=================================+   
    // | Elementwise Comparison          |
    // +=================================+

    VkVec operator>(const float_t value) const;
    VkVec operator>=(const float_t value) const;
    VkVec operator==(const float_t value) const;
    VkVec operator!=(const float_t value) const;
    VkVec operator<(const float_t value) const;
    VkVec operator<=(const float_t value) const;
    VkVec operator>(const VkVec& other) const;
    VkVec operator>=(const VkVec& other) const;
    VkVec operator==(const VkVec& other) const;
    VkVec operator!=(const VkVec& other) const;
    VkVec operator<(const VkVec& other) const;
    VkVec operator<=(const VkVec& other) const;

    // +=================================+   
    // | Elementwise Logial Operations   |
    // +=================================+

    VkVec operator&&(const bool value) const;
    VkVec operator||(const bool value) const;
    VkVec operator!() const;
    VkVec operator&&(const VkVec& other) const;
    VkVec operator||(const VkVec& other) const;

    // +=================================+   
    // | Dynamic Handling & Conversion   |
    // +=================================+

    float_t pop_first();
    float_t pop_last();
    VkVec flatten() const;
    VkVec erase_row(const uint32_t row_index);
    VkVec erase_col(const uint32_t col_index);
    VkVec erase_depth(const uint32_t depth_layer_index);
    VkVec add_rows(const int32_t rows = 1, float_t init_value = 0.0f) const;
    VkVec add_cols(const int32_t cols = 1, float_t init_value = 0.0f) const;
    VkVec add_depth(const int32_t layers = 1, float_t init_value = 0.0f) const;
    VkVec resize(const uint32_t rows, const uint32_t cols = 1, const uint32_t depth = 1, float_t init_value = 0.0f) const;
    VkVec concatenate(const VkVec& other, const uint32_t axis = 0) const;
    VkVec padding(const uint32_t amount = 1, const float_t value = 0.0f) const;
    VkVec padding_pre(const uint32_t amount = 1, const float_t value = 0.0f) const;
    VkVec padding_post(const uint32_t amount = 1, const float_t value = 0.0f) const;
    VkVec stationary() const;
    VkVec stationary_log() const;
    VkVec stationary_fract(float_t degree, float_t exponent = 2.0f) const;
    VkVec sort() const;
    VkVec pool_max(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_maxabs(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_min(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_mean(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec convolution(const VkVec& filter, bool padding = false) const;
    VkVec transpose() const;
    VkVec inverse(uint32_t iterations = 20) const;
    VkVec mirror(bool mirror_rows = true, bool mirror_cols = false, bool mirror_depth = false) const;
    VkVec diagonal() const;
    VkVec upper_trigonal() const;
    VkVec lower_trigonal() const;
    VkVec remap_to(const VkVec& target_index_map) const;

    // +=================================+   
    // | 1d vector statistics            |
    // +=================================+

    struct CorrelationResult;
    struct RegressionResult;

    CorrelationResult correlation(const VkVec& other) const;
    RegressionResult VkVec::regression(const VkVec& other, const uint32_t power = 1) const;
    VkVec rank() const;
    float_t Dickey_Fuller() const;
    float_t Engle_Granger(const VkVec& other) const;
    float_t covariance(const VkVec& other) const;

    // +=================================+   
    // | Output                          |
    // +=================================+
    
    void print(std::string comment = "", std::string delimiter = "|", bool with_indices = false, bool rows_inline = false, int32_t precision = 3) const;

protected:

    // +=================================+   
    // | Protected Class Members         |
    // +=================================+
    uint32_t rows = 0;
    uint32_t cols = 0;
    uint32_t depth = 0;
    uint32_t dimensions = 0;
	uint32_t elements = 0;
    CommandBuffer* command_buffer = nullptr;
    Buffer<float_t>* data_buffer = nullptr;
    static VkManager* manager;
    static DescriptorPool* descriptor_pool;
    
    void destroy();

    static void destroy_descriptor_pool();

    uint32_t flat_index(uint32_t row, uint32_t col = 0, uint32_t depth_layer = 0) const;

    void copy_resources(const VkVec& other);

    // +=================================+   
    // | Shader Execution (protected)    |
    // +=================================+

    void shader_exec(ShaderModule& shader, float_t& result, const float_t& constant1, const float_t& constant2 = 0, const float_t& constant3 = 0) const;
    void shader_exec(ShaderModule& shader, VkVec& result, const float_t& constant1, const float_t& constant2 = 0, const float_t& constant3 = 0) const;
    void shader_exec(ShaderModule& shader, VkVec& result, const VkVec& other, const float_t& constant1 = 0, const float_t& constant2 = 0, const float_t& constant3 = 0) const;
    void shader_exec(ShaderModule& shader, VkVec& result, const int32_t& constant1, const int32_t& constant2 = 0, const int32_t& constant3 = 0) const;
    void shader_exec(ShaderModule& shader, int32_t& result, const float_t& constant1, const float_t& constant2 = 0, const float_t& constant3 = 0) const;
    void shader_exec(ShaderModule& shader);
    void shader_exec(ShaderModule& shader, int32_t& result, const VkVec& other, const float& constant1 = 0, const float& constant2 = 0, const float& constant3 = 0) const;
    void shader_exec(ShaderModule& shader, VkVec& result, const VkVec& other1, const VkVec& other2, const float& constant1 = 0, const float& constant2 = 0, const float& constant3 = 0) const;
};

















// DEFINITIONS
// ===============================================================================================================================

// +=================================+   
// | Static Member Initializations   |
// +=================================+

DescriptorPool* VkVec::descriptor_pool = nullptr;
VkManager * VkVec::manager = nullptr;


// +=================================+   
// | Constructors & Destructors      |
// +=================================+

// parametric default constructor for multi-dimensional array:
// pass dimension size (elements per dimension)
// as uint32_t rows, cols, depth;
// row-major indexing convention applies
VkVec::VkVec(const uint32_t rows, const uint32_t cols, const uint32_t depth) {
    dimensions = depth > 1 ? 3 : cols > 1 ? 2 : rows > 0 ? 1 : 0;
    this->rows = rows;
    this->cols = cols > 0 ? cols : 1;
    this->depth = depth > 0 ? depth : 1;
    this->elements = rows * cols * depth;

    // create a shared manager for instance and device
    if (VkManager::get_singleton() == nullptr) {
        // enable instance layers
        std::vector<const char*> instance_layer_names;
        #ifdef _DEBUG
        instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
        #endif		

        // enable instance extensions
        std::vector<const char*> instance_extension_names;
        // TODO: push_back required instance extensions here

        // enable device extensions
        std::vector<const char*> device_extension_names;
        device_extension_names.push_back("VK_EXT_descriptor_indexing");
        device_extension_names.push_back("VK_EXT_shader_atomic_float");
        device_extension_names.push_back("VK_KHR_storage_buffer_storage_class");
        device_extension_names.push_back("VK_EXT_shader_image_atomic_int64");
        device_extension_names.push_back("VK_KHR_shader_non_semantic_info");

        manager = VkManager::make_singleton(
            instance_layer_names,
            instance_extension_names,
            device_extension_names,
            APPLICATION_NAME,
            APPLICATION_MAJOR_VERSION,
            APPLICATION_MINOR_VERSION,
            APPLICATION_PATCH_VERSION,
            DEFAULT_DEVICE_ID
        );

        // initialize (=seed) random number generator
        srand(time(NULL));
    }
    else {
        manager = VkManager::get_singleton();
    }

    if (descriptor_pool == nullptr) {
        descriptor_pool = new DescriptorPool(manager->get_device(), max_sets_within_pool);
        std::atexit(&VkVec::destroy_descriptor_pool);
    }

    // add a command buffer + data buffer
    if (command_buffer == nullptr) {
        command_buffer = new CommandBuffer(manager->get_device(), QueueFamily::COMPUTE, manager->get_command_pool_compute());
    }
    if (data_buffer != nullptr) {
        delete this->data_buffer;
    }

    VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // allocate as a 'flat' buffer (3d converted to 1d) -> this is required because GLSL shaders only support dynamic sizing in a single (=the last) dimension
    data_buffer = new Buffer<float_t>(manager->get_device(), BufferUsage::STORAGE, this->elements, 1, 1, memory_properties);
}

// move constructor
VkVec::VkVec(VkVec&& other) noexcept {
    copy_resources(other);
    if (this->command_buffer == nullptr) {
        this->command_buffer = new CommandBuffer(manager->get_device(), QueueFamily::COMPUTE, manager->get_command_pool_compute());
    }
    this->command_buffer = other.command_buffer; other.command_buffer = nullptr;
    this->data_buffer = other.data_buffer; other.data_buffer = nullptr;
}

// VkVec copy constructor
VkVec::VkVec(const VkVec& other) {
    copy_resources(other);
    if (this->command_buffer == nullptr) {
        this->command_buffer = new CommandBuffer(manager->get_device(), QueueFamily::COMPUTE, manager->get_command_pool_compute());
    }

    if (data_buffer != nullptr) {
        delete this->data_buffer;
    }

    VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    data_buffer = new Buffer<float_t>(manager->get_device(), BufferUsage::STORAGE, this->elements, 1, 1, memory_properties);
    this->data_buffer = other.data_buffer;
}

// protected helper method of object destruction
void VkVec::destroy() {
    if (this->data_buffer != nullptr) {
        delete this->data_buffer;
        this->data_buffer = nullptr;
    }
    if (this->command_buffer) {
        delete this->command_buffer;
        this->command_buffer = nullptr;
    }
}

// destructor
VkVec::~VkVec() {
    this->destroy();
}

// protected helper method for copying simple class member variables
void VkVec::copy_resources(const VkVec& other) {
    this->elements = other.elements;
    this->rows = other.rows;
    this->cols = other.cols;
    this->depth = other.depth;
    this->dimensions = other.dimensions;
    this->manager = other.manager;
}

// +=================================+   
// | getters & setters               |
// +=================================+

// assigns a value to a data element via its index
void VkVec::set(const float_t value, const uint32_t row, const uint32_t col, const uint32_t depth) {
    // using flat index as 'row' index
    this->data_buffer->set(value, row * this->cols * this->depth + col * this->depth + depth);
}

// copies raw data from a std::vector<float_t> to the data buffer
// of the underlying VkVec array
void VkVec::set(std::vector<float_t>& data) {
    data_buffer->write(data);
}

// returns the value of an array element via its flattened index
float_t VkVec::get(const uint32_t row, const uint32_t col, const uint32_t depth) const {
    // using flat index as 'row' index
    return data_buffer->get(row * this->cols * this->depth + col * this->depth + depth);
}

// returns a flat copy of the raw data of the underlying buffer as type std::vector<float_t>
std::vector<float> VkVec::get() const {
    return data_buffer->read();
}

// returns the number of dimensions of the underlying array
uint32_t VkVec::get_dimensions() const {
    return this->dimensions;
}

// returns the number of rows, i.e. the size of the first dimension (indexed 0)
uint32_t VkVec::get_rows() const {
    return this->rows;
}

// returns the number of columns, i.e. the size of the second dimension (indexed 1)
uint32_t VkVec::get_cols() const {
    return this->cols;
}

// returns the array depth,
// i.e. the size of the third dimension (indexed 2)
uint32_t VkVec::get_depth() const {
    return this->depth;
}

// returns the total number of elements of the underlying array
uint32_t VkVec::get_elements() const {
    return this->elements;
}

// returns the shape of the array as std::string
std::string VkVec::get_shapestring() const {
    std::string result = "{";
    if (this->rows > 0) { result += std::to_string(this->rows); }
    if (this->cols > 1 || this->depth > 1) { result += ","; result += std::to_string(this->cols); }
    if (this->depth > 1) { result += ","; result += std::to_string(this->depth); }
    return result;
}

// returns a single row sliced from a 2d or 3d array
VkVec VkVec::get_row(int32_t row_index)  const {
    VkVec result(1, this->cols, this->depth);

    if (this->dimensions == 1) {
        Log::log(WARNING, "usage of method 'VkVec get_row(uint32_t row_index)' with a 1d array -> result contains a single scalar; ",
            "this isn't strictly invalid, but for better efficiency consider using 'float_t get(uint32_t row)' instead");
        result.set(this->get(row_index), 0);
        return result;
    }

    if (row_index >= this->rows || row_index < 0) {
        Log::log(ERROR, "invalid usage of method 'VkVec get_row(uint32_t row_index)' with invalid row index; index is ", row_index, ", the underlying array has ", this->rows, " row(s)");
    }

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_row.spv"); }
    shader_exec(shader, result, row_index);
    return result;
}

// returns a single column sliced from a 2d or 3d array
VkVec VkVec::get_col(int32_t col_index) const {
    VkVec result(this->rows, 1, this->depth);

    if (col_index >= this->cols || col_index < 0) {
        Log::log(ERROR, "invalid usage of method 'VkVec get_col(uint32_t col_index)' with invalid column index; index is ", col_index, ", the underlying array has ", this->cols, " column(s)");
    }

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_col.spv"); }
    shader_exec(shader, result, col_index);
    return result;
}

// returns a single depth layer sliced from a 3d array
VkVec VkVec::get_layer(int32_t layer_index) const {
    VkVec result(this->rows, this->cols, 1);

    if (layer_index >= this->depth || layer_index < 0) {
        Log::log(ERROR, "invalid usage of method 'VkVec get_layer(uint32_t layer_index)' with invalid depth layer index; index is ", layer_index, ", the underlying array has ", this->depth, " depth layer(s)");
    }

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_layer.spv"); }
    shader_exec(shader, result, layer_index);
    return result;
}

// +=================================+   
// | Fill, Initialize                |
// +=================================+

// fill entire array with given floating point value
void VkVec::fill(const float_t value) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// initialize the entire array with zeros
void VkVec::fill_zero() {
    this->fill(0.0f);
}

// fill entire array with identity matrix
void VkVec::fill_identity() {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_identity.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with values from a random normal (=gaussian) distribution
void VkVec::fill_random_gaussian(const float_t mu, const float_t sigma) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_gaussian.spv"); }
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_value(seed32());
    push_constants.add_value(mu);
    push_constants.add_value(sigma);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with values from a random uniform distribution
void VkVec::fill_random_uniform(const float_t min, const float_t max) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_uniform.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(min);
    push_constants.add_value(max);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with values from a random uniform distribution
void VkVec::fill_random_uniform_int(const int32_t min, const int32_t max) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_uniform_int.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(min);
    push_constants.add_value(max);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// randomly sets the specified fraction of the values to zero and the rest to 1 (default: 0.5, i.e. 50%)
void VkVec::fill_random_binary(float_t ratio) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_binary.spv"); }

    // check valid ratio
    if (ratio > 1 || ratio < 0) {
        Log::log(WARNING,
            "invalid usage of method 'void VkVec::fill_binary(float_t ratio)': "
            "ratio argument must be between 0-1 but is ", ratio,
            " --> argument will be clipped to fit this range");
    }
    float_t valid_ratio = std::fmax(std::fmin(ratio, 1.0f), 0);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(valid_ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// randomly sets the specified fraction of the values to -1 and the rest to +1 (default: 0.5, i.e. 50%)
void VkVec::fill_random_sign(float_t ratio) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_sign.spv"); }

    // check valid ratio
    if (ratio > 1 || ratio < 0) {
        Log::log(WARNING,
            "invalid usage of method 'void VkVec::fill_sign(float_t ratio)': "
            "ratio argument must be between 0-1 but is ", ratio,
            " --> argument will be clipped to fit this range");
    }
    float_t valid_ratio = std::fmax(std::fmin(ratio, 1.0f), 0);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(valid_ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}


// fills the array with a continuous
// range of numbers (with specified start parameter
// referring to the zero position and a step parameter)
// in all dimensions
void VkVec::fill_range(const float_t start, const float_t step) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_range.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->rows);
    push_constants.add_value(this->cols);
    push_constants.add_value(this->depth);
    push_constants.add_value(start);
    push_constants.add_value(step);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

void VkVec::fill_dropout(float_t ratio) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_dropout.spv"); }

    // check valid ratio
    if (ratio > 1 || ratio < 0) {
        Log::log(WARNING,
            "invalid usage of method 'void VkVec::fill_dropout(float_t ratio)': "
            "ratio argument must be between 0-1 but is ", ratio,
            " --> argument will be clipped to fit this range");
    }
    float_t valid_ratio = std::fmax(std::fmin(ratio, 1.0f), 0);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with normal "Xavier" weight initialization
// (by Xavier Glorot & Bengio) for tanh activation
void VkVec::fill_Xavier_normal(uint32_t fan_in, uint32_t fan_out) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_normal.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(fan_in);
    push_constants.add_value(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with uniform "Xavier" weight initializiation
// (by Xavier Glorot & Bengio), e.g. for tanh activation
void VkVec::fill_Xavier_uniform(uint32_t fan_in, uint32_t fan_out) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_uniform.spv"); }

    float_t seed = (float_t)(double(rand()) / RAND_MAX);

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed);
    push_constants.add_value(fan_in);
    push_constants.add_value(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with uniform "Xavier" weight initialization
// for sigmoid activation
void VkVec::fill_Xavier_sigmoid(uint32_t fan_in, uint32_t fan_out) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_sigmoid.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(fan_in);
    push_constants.add_value(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with "Kaiming He" normal weight initialization,
// used for ReLU activation
void VkVec::fill_He_ReLU(uint32_t fan_in) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_He_ReLU.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(fan_in);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// fill with modified "Kaiming He" nornal weight initialization,
// used for ELU activation
void VkVec::fill_He_ELU(uint32_t fan_in) {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_He_ELU.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_value(this->elements);
    push_constants.add_value(seed32());
    push_constants.add_value(fan_in);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
}

// +=================================+   
// | Distribution Properties         |
// +=================================+

// returns the lowest value of the VkVec,
// across all dimensions
float_t VkVec::min() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min.spv"); }
    Buffer<float> result(manager->get_device(), STORAGE, this->elements);
    Buffer<uint32_t> finished_workgroups(manager->get_device(), STORAGE, 1);
    finished_workgroups.set(0, 0);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(finished_workgroups, 2);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    TIMER;
    command_buffer->compute(pipeline, this->elements, 1, 1, 128); TIMER_STOP;
    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// returns the highest value of the VkVec,
// across all dimensions
float_t VkVec::max() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max.spv"); }
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> finished_workgroups(manager->get_device(), STORAGE, 1);
    finished_workgroups.set(0, 0);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(finished_workgroups, 2);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// returns the value of the VkVec with the highest
// deviation from zero, across all dimensions
float_t VkVec::maxabs() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("maxabs.spv"); }
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> finished_workgroups(manager->get_device(), STORAGE, 1);
    finished_workgroups.set(0, 0);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(finished_workgroups, 2);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// returns the arrithmetic mean of all values of the VkVec
float_t VkVec::mean() const {
    return this->sum() / this->elements;
}

// returns the median of all values the VkVec;
// VkVec must be 1d
float_t VkVec::median() const {

    // confirm 1d array
    if (this->dimensions > 1) {
        Log::log(ERROR, "invalid usage of method VkVec::median(), underlying array must be 1d but has ", this->dimensions, " dimensions");
    }

    VkVec sorted = this->sort();
    // odd number of elements
    if (this->rows % 2) {
        return sorted.get(elements / 2);
    }
    // even number of elements
    else {
        return (sorted.get(elements / 2 - 1) + sorted.get(elements / 2)) / 2;
    }
}

// returns the variance of all values of a vector, matrix or array
// as a floating point number
float_t VkVec::variance() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("variance.spv"); }

    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> finished_workgroups(manager->get_device(), STORAGE, 2);
    finished_workgroups.set(0, 0);
    finished_workgroups.set(0, 1);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(finished_workgroups, 2);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// returns the standard deviation of all values a the vector, matrix or array
float_t VkVec::stddev() const {
    return std::sqrt(this->variance());
}

// returns the skewness of all data of the VkVec
// across all dimensions
float_t VkVec::skewness() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("skewness.spv"); }

    Buffer<float> mdev2(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev3(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, 1);
    Buffer<uint32_t> finished_workgroups(manager->get_device(), STORAGE, 3);
    finished_workgroups.set(0, 0);
    finished_workgroups.set(0, 1);
    finished_workgroups.set(0, 2);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(mdev2, 1);
    descriptor_set.bind_buffer(mdev3, 2);
    descriptor_set.bind_buffer(result, 3);
    descriptor_set.bind_buffer(finished_workgroups, 4);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 128);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// returns the kurtosis of all data of the VkVec
// across all dimensions
float_t VkVec::kurtosis() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("kurtosis.spv"); }

    Buffer<float> mdev2(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev4(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, 1);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(mdev2, 1);
    descriptor_set.bind_buffer(mdev4, 2);
    descriptor_set.bind_buffer(result, 3);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}


// +=================================+   
// | Addition                        |
// +=================================+

// returns the sum of all array elements;
float_t VkVec::sum() const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sum.spv"); }
    Buffer<uint32_t> finished_workgroups(manager->get_device(), BufferUsage::STORAGE, 1);
    finished_workgroups.set(0, 0);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(finished_workgroups, 2);

    PushConstants push_constants;
    push_constants.add_value(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
    return result.get(0);
}

// elementwise addition of the specified value to all values of the array
VkVec VkVec::operator+(const float_t value) const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_plus_value.spv"); }
    VkVec result(this->rows, this->cols, this->depth);
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

// returns the resulting array of the elementwise addition of two arrays
VkVec VkVec::operator+(const VkVec& other) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_plus_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// prefix increment operator;
// increments the values of the array by +1,
// returns a reference to the source array itself
VkVec& VkVec::operator++() {
    *this += 1.0f;
    return *this;
}

// postfix increment operator;
// makes an internal copy of the array,
// then increments all values of the array by +1,
// then returns the temporary copy;
// note: more overhead then with the prefix increment
// because of extra copy!
VkVec VkVec::operator++(int) {
    VkVec copy(this->rows, this->cols, this->depth);
    copy = *this;
    *this += 1.0f;
    return copy;
}

// elementwise addition of the specified
// value to the elements of the array
void VkVec::operator+=(const float_t value) {
    *this = this->operator+(value);
}

// elementwise addition of the values of 'other'
// to the values of the corresponding elements of 'this'
void VkVec::operator+=(const VkVec& other) {
    *this = this->operator+(other);
}


// +=================================+   
// | Substraction                    |
// +=================================+

// elementwise substraction of the specified value from all values of the array
VkVec VkVec::operator-(const float_t value) const {
    // using the member method "VkVec operator+(const float_t value) const"
    return this->operator+(value * -1);
}

// returns the resulting array of the elementwise substraction of
// two array of equal dimensions
VkVec VkVec::operator-(const VkVec& other) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_minus_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// prefix decrement operator;
// decrements the values of the array by -1
VkVec& VkVec::operator--() {
    *this = *this + (-1.0f);
    return *this;
}

// postfix decrement operator;
// makes an internal copy of the array,
// then decrements all values of the array by -1,
// then returns the temporary copy;
// note: more overhead then with the prefix decrement
// because of extra copy!
VkVec VkVec::operator--(int) {
    VkVec copy(this->rows, this->cols, this->depth);
    copy= *this;
    *this = *this + (-1.0f);
    return copy;
}

// elementwise substraction of the specified
// value from the elements of the array
void VkVec::operator-=(const float_t value) {
    *this = *this + (value * -1);
}

// elementwise substraction of the values of 'other'
// from the values of the corresponding elements of 'this'
void VkVec::operator-=(const VkVec& other) {
    *this = this->operator-(other);
}

// +=================================+   
// | Multiplication                  |
// +=================================+

// returns the product reduction, i.e. the result
// of multiplication all individual elements of the array
float_t VkVec::product() const {
    auto as_stdvec = this->data_buffer->read();
    float product = as_stdvec[0];
    for (uint32_t i = 1; i < elements; i++) {
        product *= as_stdvec[i];
    }
    return product;
}

// elementwise multiplication with a scalar
VkVec VkVec::operator*(const float_t factor) const {
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_multiply_factor.spv"); }
    VkVec result(this->rows, this->cols, this->depth);
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise multiplication (*=) with a scalar
void VkVec::operator*=(const float_t factor) {
    *this = this->operator*(factor);
}

// Alias for 2D or 3D matrix multiplication
VkVec VkVec::operator*(const VkVec& other) const {
    return this->matrix_product(other);
}

// Alias for tensordot matrix multiplication;
// note: 'this' is getting reassigned and may change its shape as a consequence of this operation
void VkVec::operator*=(const VkVec& other) {
    *this = this->matrix_product(other);
}

// scalar product
float_t VkVec::scalar_product(const VkVec& other) const {
    return this->Hadamard_product(other).sum();
}

// 2D or 3d matrix dotproduct
VkVec VkVec::matrix_product(const VkVec& other) const {
    VkVec result(this->rows, other.get_cols(), this->depth);
    if (other.get_rows() != this->cols || other.get_cols() != this->rows || other.get_depth() != this->depth) {
        Log::log(WARNING, "invalid usage of method VkVec::dotproduct() for matrix dotproduct: 'this' has shape {", this->rows, ",", this->cols, ",",
            this->depth, "}, therefore 'other' must have size {", this->cols, ",", this->rows, ",", this->depth, "} but has ",
            other.get_shapestring(), "; result is undefined");
        return result;
    }
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("matrix_product.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// elementwise multiplication of the values of the current
// array with the corresponding values of a second array,
// resulting in the 'Hadamard product';
// the dimensions of the two arrays must match!
VkVec VkVec::Hadamard_product(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("hadamard_product.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// +=================================+   
// | Division                        |
// +=================================+

// elementwise division by a scalar
VkVec VkVec::operator/(const float_t quotient) const {
    if (quotient == 0) {
        Log::log(ERROR,
            "invalid call of method 'VkVec VkVec::operator/(const T quotient)' ",
            "with quotient=0 (zero division is undefined)");
    }
    return (*this) * (1.0f / quotient);
}

// elementwise division (/=) by a scalar
void VkVec::operator/=(const float_t quotient) {
    (*this) *= (1.0f / quotient);
}

// elementwise division of the values of the current
// array by the corresponding values of a second VkVec,
// resulting in the 'Hadamard division';
// the dimensions of the two arrays must match!
VkVec VkVec::Hadamard_division(const VkVec& other) {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("hadamard_division.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// +=================================+   
// | Modulo                          |
// +=================================+

// elementwise modulo operation, converting the VkVec values
// to the remainders of their division by the specified number
void VkVec::operator%=(const float_t value) {
    *this = this->operator%(value);
}

// elementwise modulo operation, resulting in an VkVec array that
// contains the remainders of the division of the values of
// the original array by the specified number
VkVec VkVec::operator%(const float_t value) const {
    if (value == 0) {
        Log::log(WARNING,
            "invalid usage of method 'VkVec VkVec::operator%(const float_t value) const' ",
            "with value=0 (zero division is undefined) --> 'this' will remain unmodified");
        return *this;
    }
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_modulo_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Exponentiation & Logarithm      |
// +=================================+

// elementwise exponentiation to the power of
// the specified exponent
VkVec VkVec::pow(const float_t exponent) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pow.spv"); }
    shader_exec(shader, result, exponent); // OVERLOAD_2
    return result;
}

// alias for pow(exponent):
// elementwise exponentiation to the power of
// the specified exponent
VkVec VkVec::operator^(const float_t exponent) const {
    return this->pow(exponent);
}

// alias for pow(other):
// elementwise exponentiation to the power of
// the corresponding element of 'other'
VkVec VkVec::operator^(const VkVec& other) const {
    return this->pow(other);
}

// elementwise exponentiation of the values of 'this'
// to the power of the specified exponent
void VkVec::operator^=(const float_t exponent) {
    *this = this->pow(exponent);
}

// elementwise exponentiation to the power of
// the corresponding values of the second array;
// the dimensions of the two array must match!
VkVec VkVec::pow(const VkVec& other) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pow_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// converts the individual values of the array
// elementwise to their square root
VkVec VkVec::sqrt() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sqrt.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

VkVec VkVec::log(float_t base) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("log.spv"); }
    shader_exec(shader, result, base); // OVERLOAD_2
    return result;
}

VkVec VkVec::exp() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("exp.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Rounding                        |
// +=================================+

// rounds the values of the array elementwise
// to their nearest integers
VkVec VkVec::round() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("round.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// rounds the values of the array elementwise
// to their next lower integers
VkVec VkVec::floor() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("floor.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// returns a copy of the array that stores the values as rounded
// to their next higher integers
VkVec VkVec::ceil() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("ceil.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// returns a copy of the array that stores the
// absolute values of the source array
VkVec VkVec::abs() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("abs.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Min, Max                        |
// +=================================+

// elementwise minimum of the specified value
// and the data elements of the array
VkVec VkVec::min(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

// elementwise maximum of the specified value
// and the data elements of the array
VkVec VkVec::max(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

// returns the result of elementwise min() comparison
// of 'this' vs 'other'
VkVec VkVec::min(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// returns the result of elementwise max() comparison
// of 'this' vs 'other'
VkVec VkVec::max(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// +=================================+   
// | Trigonometric Functions         |
// +=================================+

// elementwise application of the cos() function
VkVec VkVec::cos(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("cos.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the sin() function
VkVec VkVec::sin(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sin.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the tan function
VkVec VkVec::tan(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tan.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the acos() function
VkVec VkVec::acos(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("acos.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the asin() function
VkVec VkVec::asin(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("asin.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the atan function
VkVec VkVec::atan(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("atan.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the hyperbolic cosine function
VkVec VkVec::cosh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("cosh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise applicatiohn of the hyperbolic sine function
VkVec VkVec::sinh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sinh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the hyperbolic tangent function
VkVec VkVec::tanh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tanh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the hyperbolic arc cosine function
VkVec VkVec::acosh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("acosh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the hyperbolic arc sine function
VkVec VkVec::asinh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("asinh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// elementwise application of the hyperbolic arc tangent function
VkVec VkVec::atanh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("atanh.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Find, Replace                   |
// +=================================+

// searches the array buffer for the specified 'old_value' and
// replaces all occurrences by the 'new_value';
// in order to mitigate floating point number rounding imprecisions,
// this method will consider any values that no more than
// epsilon = 0.0000001 from the old value as a match
VkVec VkVec::replace(const float_t& old_value, const float_t& new_value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace.spv"); }
    shader_exec(shader, result, old_value, new_value); // OVERLOAD_2
    return result;
}

// replaces all elements of 'this' with the corresponding element of the
// 'replacing_map' if the corresponding element of the condition map is !=0
VkVec VkVec::replace_if(const VkVec& condition_map, const VkVec& replacing_map) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace_if_other.spv"); }
    shader_exec(shader, result, condition_map, replacing_map); // OVERLOAD_8
    return result;
}

// replaces all elements of 'this' with the corresponding element of the
// 'replacing_map' if the corresponding element of the condition map is !=0
VkVec VkVec::replace_if(const VkVec& condition_map, const float_t replacing_value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace_if_value.spv"); }
    shader_exec(shader, result, condition_map, replacing_value); // OVERLOAD_3
    return result;
}

// returns the number of occurrences of the specified value;
// in order to mitigate floating point number rounding imprecisions,
// this method will consider any values that no more than
// epsilon = 0.0000001 from the old value as a match
uint32_t VkVec::find(const float_t& value) const {
    static int32_t result; result = 0;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("find.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_5
    return result;
}

// returns a VkVec array of equal dimensions as the source,
// with -1 for all corresponding negative values and +1 for all corresponding positive values
// (0 for all zeros)
VkVec VkVec::sign() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sign.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Scaling                         |
// +=================================+

// scale to specified range
VkVec VkVec::scale_minmax(float_t range_from, float_t range_to) const {
    float_t data_min = this->min();
    float_t data_max = this->max();
    float_t scale_factor = (range_to - range_from) / (data_max - data_min);
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("scale_minmax.spv"); }
    shader_exec(shader, result, range_from, data_min, scale_factor); // OVERLOAD_2
    return result;
}

// mean normalization scaling, i.e.
// (x - mean) / (max - min)
VkVec VkVec::scale_mean() const {
    float_t data_min = this->min();
    float_t data_max = this->max();
    float_t data_mean = this->mean();
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("scale_mean.spv"); }
    shader_exec(shader, result, data_min, data_max, data_mean); // OVERLOAD_2
    return result;
}

// scaling to zero mean and unit-variance, i.e.
// (x - mean) / sigma
VkVec VkVec::scale_standardized() const {
    static float_t mean, stddev, sum_of_squares;
    mean = this->mean();

    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader1(manager->get_device());
    if (!shader1.get()) { shader1.read_from_file("mdev_squared.spv"); }
    shader_exec(shader1, result, mean); // OVERLOAD_2
    sum_of_squares = result.sum();
    stddev = std::sqrt(sum_of_squares / this->elements);

    static ShaderModule shader2(manager->get_device());
    if (!shader2.get()) { shader2.read_from_file("scale_standardized.spv"); }
    shader_exec(shader2, result, mean, stddev); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Activation Functions            |
// +=================================+

VkVec VkVec::activation(ActFunc activation_function) const {
    switch (activation_function) {
    case ActFunc::RELU:
        return this->relu(0.0f);
        break;
    case ActFunc::LRELU:
        return this->relu(0.01f);
        break;
    case ActFunc::ELU:
        return this->elu(0.0f);
        break;
    case ActFunc::LELU:
        return this->elu(0.01f);
        break;
    case ActFunc::SIGMOID:
        return this->sigmoid();
        break;
    case ActFunc::TANH:
        return this->tanh();
        break;
    case ActFunc::IDENT:
        return this->ident();
        break;
    default:
        return *this;
        break;
    }
}

VkVec VkVec::derivative(ActFunc activation_function) const {
    switch (activation_function) {
    case ActFunc::RELU:
        return this->relu_drv(0.0f);
        break;
    case ActFunc::LRELU:
        return this->relu_drv(0.01f);
        break;
    case ActFunc::ELU:
        return elu_drv(0.0f);
        break;
    case ActFunc::LELU:
        return elu_drv(0.01f);
        break;
    case ActFunc::SIGMOID:
        return sigmoid_drv();
        break;
    case ActFunc::TANH:
        return tanh_drv();
        break;
    case ActFunc::IDENT:
        return ident_drv();
        break;
    default:
        return *this;
        break;
    }
}

// identity activation function
VkVec VkVec::ident() const {
    VkVec result;
    result = *this; // copy constructor invocation
    return result;
}

// identity activation function derivative
VkVec VkVec::ident_drv() const {
    VkVec result(this->rows, this->cols, this->depth);
    result.fill(1.0f);
    return result;
}

// sigmoid activation function
// 1/(1+exp(-x))
VkVec VkVec::sigmoid() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sigmoid.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// sigmoid activation derivative
// exp(x)/pow(exp(x)+1,2)
VkVec VkVec::sigmoid_drv() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sigmoid_drv.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// ELU activation function
VkVec VkVec::elu(float_t alpha) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("elu.spv"); }
    shader_exec(shader, result, alpha); // OVERLOAD_2
    return result;
}

// ELU activation derivative;
// chose alpha=0 for true ELU function;
// small alpha value like e.g. 0.01 for 'leaky' ELU
VkVec VkVec::elu_drv(float_t alpha) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("elu_drv.spv"); }
    shader_exec(shader, result, alpha); // OVERLOAD_2
    return result;
}


// ReLU activation function;
// chose alpha=0 for true ReLU function;
// small alpha value like e.g. 0.01 for 'leaky' ReLU
VkVec VkVec::relu(float_t alpha) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("relu.spv"); }
    shader_exec(shader, result, alpha); // OVERLOAD_2
    return result;
}

// ReLU activation derivative;
// chose alpha=0 for true ReLU function;
// small alpha value like e.g. 0.01 for 'leaky' ReLU
VkVec VkVec::relu_drv(float_t alpha) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("relu_drv.spv"); }
    shader_exec(shader, result, alpha); // OVERLOAD_2
    return result;
}

// tanh activation derivative
VkVec VkVec::tanh_drv(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tanh_drv.spv"); }
    shader_exec(shader, result, factor); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Outlier Treatment               |
// +=================================+

// truncate outliers by z-score mean deviation
VkVec VkVec::outliers_truncate(float_t z_score) const {
    static float_t mean, stddev, sum_of_squares, lower_margin, upper_margin = 0;
    
    mean = this->mean();
    
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader1(manager->get_device());
    if (!shader1.get()) { shader1.read_from_file("mdev_squared.spv"); }
    shader_exec(shader1, result, mean); // OVERLOAD_2
    sum_of_squares = result.sum();
    stddev = std::sqrt(sum_of_squares / this->elements);

    lower_margin = mean - z_score * stddev;
    upper_margin = mean + z_score * stddev;
    
    static ShaderModule shader2(manager->get_device());
    if (!shader2.get()) { shader2.read_from_file("outliers_truncate.spv"); }
    shader_exec(shader2, result, lower_margin, upper_margin); // OVERLOAD_2
    return result;
}

// set outliers (by z-score) to mean
VkVec VkVec::outliers_mean_imputation(float_t z_score) const {
    static float_t mean, stddev, sum_of_squares, lower_margin, upper_margin = 0;

    mean = this->mean();

    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader1(manager->get_device());
    if (!shader1.get()) { shader1.read_from_file("mdev_squared.spv"); }
    shader_exec(shader1, result, mean); // OVERLOAD_2
    sum_of_squares = result.sum();
    stddev = std::sqrt(sum_of_squares / this->elements);

    lower_margin = mean - z_score * stddev;
    upper_margin = mean + z_score * stddev;

    static ShaderModule shader2(manager->get_device());
    if (!shader2.get()) { shader2.read_from_file("outliers_value_imputation.spv"); }
    shader_exec(shader2, result, lower_margin, upper_margin, mean); // OVERLOAD_2
    return result;
}

// set outliers (by z-score) to value
VkVec VkVec::outliers_value_imputation(float_t value, float_t z_score) const {
    static float_t mean, stddev, sum_of_squares, lower_margin, upper_margin = 0;

    mean = this->mean();

    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader1(manager->get_device());
    if (!shader1.get()) { shader1.read_from_file("mdev_squared.spv"); }
    shader_exec(shader1, result, mean); // OVERLOAD_2
    sum_of_squares = result.sum();
    stddev = std::sqrt(sum_of_squares / this->elements);

    lower_margin = mean - z_score * stddev;
    upper_margin = mean + z_score * stddev;

    static ShaderModule shader2(manager->get_device());
    if (!shader2.get()) { shader2.read_from_file("outliers_value_imputation.spv"); }
    shader_exec(shader2, result, lower_margin, upper_margin, value); // OVERLOAD_2
    return result;
}

// recover -inf, +inf or nan values
VkVec VkVec::recover() const {
    float_t seed = (float_t)(double(rand()) / RAND_MAX);
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("recover.spv"); }
    shader_exec(shader, result, seed); // OVERLOAD_2
    return result;
}

// +=================================+   
// | Assignment                      |
// +=================================+

// copy assignment
VkVec& VkVec::operator=(const VkVec& other) {
    // Check for self-assignment
    if (this != &other) {
        this->elements = other.elements;
        this->rows = other.rows;
        this->cols = other.cols;
        this->depth = other.depth;
        this->dimensions = other.dimensions;
        *this->data_buffer = *other.data_buffer;
    }
    return *this;
}

// move assignment
VkVec& VkVec::operator=(VkVec&& other) noexcept {
    // Check for self-assignment
    if (this != &other) {
        // Move data from other object
        this->elements = other.elements;
        this->rows = other.rows;
        this->cols = other.cols;
        this->depth = other.depth;
        this->dimensions = other.dimensions;
        *this->data_buffer = std::move(*other.data_buffer);
    }
    return *this;
}

// +=================================+   
// | Elementwise Comparison          |
// +=================================+

VkVec VkVec::operator>(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greater_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}


VkVec VkVec::operator>=(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greaterequals_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

VkVec VkVec::operator==(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("equals_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

VkVec VkVec::operator!=(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("notequals_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

VkVec VkVec::operator<(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("less_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

VkVec VkVec::operator<=(const float_t value) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lessequals_value.spv"); }
    shader_exec(shader, result, value); // OVERLOAD_2
    return result;
}

// elementwise comparison with second VkVec
VkVec VkVec::operator>(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greater_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

VkVec VkVec::operator>=(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greaterequals_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

VkVec VkVec::operator==(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("equals_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

VkVec VkVec::operator!=(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("notequals_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}
VkVec VkVec::operator<(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("less_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

VkVec VkVec::operator<=(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lessequals_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}


// +=================================+   
// | Elementwise Logial Operations   |
// +=================================+

// elementwise logical 'and'
VkVec VkVec::operator&&(const bool value) const {
    if (value == false) {
        VkVec result(this->rows, this->cols, this->depth);
        result.fill(0);
        return result;
    }
    else {
        return this->operator!=(0.0f);
    }
}

// elementwise logical 'or'
VkVec VkVec::operator||(const bool value) const {
    if (value == true) {
        VkVec result(this->rows, this->cols, this->depth);
        result.fill(1);
        return result;
    }
    else {
        return this->operator!=(0.0f);
    }
}

// elementwise 'not'
VkVec VkVec::operator!() const {
    return this->operator==(0.0f);
}

VkVec VkVec::operator&&(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("and_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

VkVec VkVec::operator||(const VkVec& other) const {
    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("or_other.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// +=================================+   
// | dynamic handling                |
// +=================================+

// conversion from 2d or 3d array to 1d vector
VkVec VkVec::flatten() const {
    VkVec result(this->elements);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("flatten.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// add the specified number of rows;
// a default value can be passed for initialization of any newly added elements;
// a negative number of added rows can be used to remove rows
VkVec VkVec::add_rows(const int32_t rows, float_t init_value) const {
    return this->resize(this->rows + rows, this->cols, this->depth, init_value);
}

// add the specified number of columns;
// a default value can be passed for initialization of any newly added elements;
// a negative number of added columns can be used to remove columns
VkVec VkVec::add_cols(const int32_t cols, float_t init_value) const {
    return this->resize(this->rows, this->cols + cols, this->depth, init_value);
}

// add the specified depth layers to the z dimension;
// a default value can be passed for initialization of any newly added elements;
// a negative number of added depth can be used to remove a number of z dimension layers
VkVec VkVec::add_depth(const int32_t layers, float_t init_value) const {
    return this->resize(this->rows, this->cols, this->depth + layers, init_value);
}

// resizes the underlying array buffer to the specified dimensions;
// any new elements get initialized to the given value (default: 0)
VkVec VkVec::resize(const uint32_t rows, const uint32_t cols, const uint32_t depth, float_t init_value) const {
    VkVec result(rows, cols, depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("resize.spv"); }
    shader_exec(shader, result, init_value); // OVERLOAD_2
    return result;
}

// stitch two VkVec arrays together along the specified axis
VkVec VkVec::concatenate(const VkVec& other, const uint32_t axis) const {
    if (axis > 2) {
        Log::log(ERROR, "in method VkVec::concatenate() invalid axis argument (is ", axis, " but no values > 2 are allowed");
    }
    VkVec result(
        axis == 0 ? this->rows + other.get_rows() : this->rows,
        axis == 1 ? this->cols + other.get_cols() : this->cols,
        axis == 2 ? this->depth + other.get_depth() : this->depth
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("concatenate.spv"); }
    shader_exec(shader, result, other); // OVERLOAD_3
    return result;
}

// padding in all directions (before and after rows + cols)
VkVec VkVec::padding(const uint32_t amount, const float_t value) const {
    VkVec result(
        this->rows > 1 ? this->rows + 2 * amount : this->rows,
        this->cols > 1 ? this->cols + 2 * amount : this->cols,
        this->depth
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("padding.spv"); }
    shader_exec(shader, result, float_t(amount), value); // OVERLOAD_2
    return result;
}

// add padding before all rows + cols
VkVec VkVec::padding_pre(const uint32_t amount, const float_t value) const {
    VkVec result(
        this->rows > 1 ? this->rows + amount : this->rows,
        this->cols > 1 ? this->cols + amount : this->cols,
        this->depth
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("padding.spv"); }
    shader_exec(shader, result, float_t(amount), value); // OVERLOAD_2
    return result;
}

VkVec VkVec::padding_post(const uint32_t amount, const float_t value) const {
    return this->resize(
        this->rows > 1 ? this->rows + amount : this->rows,
        this->cols > 1 ? this->cols + amount : this->cols,
        this->depth,
        value
    );
}

VkVec VkVec::pool_max(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    VkVec result(
        std::max((int(this->rows) - (slider_rows - 1)) / slider_rows, 1),
        std::max((int(this->cols) - (slider_cols - 1)) / slider_cols, 1),
        std::max((int(this->depth) - (slider_depth - 1)) / slider_depth, 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_max.spv"); }
    shader_exec(shader, result, slider_rows, slider_cols, slider_depth); // OVERLOAD_4
    return result;
}

VkVec VkVec::pool_maxabs(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    VkVec result(
        std::max((int(this->rows) - (slider_rows - 1)) / slider_rows, 1),
        std::max((int(this->cols) - (slider_cols - 1)) / slider_cols, 1),
        std::max((int(this->depth) - (slider_depth - 1)) / slider_depth, 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_maxabs.spv"); }
    shader_exec(shader, result, slider_rows, slider_cols, slider_depth); // OVERLOAD_4
    return result;
}

VkVec VkVec::pool_min(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    VkVec result(
        std::max((int(this->rows) - (slider_rows - 1)) / slider_rows, 1),
        std::max((int(this->cols) - (slider_cols - 1)) / slider_cols, 1),
        std::max((int(this->depth) - (slider_depth - 1)) / slider_depth, 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_min.spv"); }
    shader_exec(shader, result, slider_rows, slider_cols, slider_depth); // OVERLOAD_4
    return result;
}

VkVec VkVec::pool_mean(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    VkVec result(
        std::max((int(this->rows) - (slider_rows - 1)) / slider_rows, 1),
        std::max((int(this->cols) - (slider_cols - 1)) / slider_cols, 1),
        std::max((int(this->depth) - (slider_depth - 1)) / slider_depth, 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_mean.spv"); }
    shader_exec(shader, result, slider_rows, slider_cols, slider_depth); // OVERLOAD_4
    return result;
}

VkVec VkVec::convolution(const VkVec& filter, bool padding) const {
    VkVec result(
        padding ? this->rows : this->rows - filter.get_rows() + 1,
        padding ? this->cols : this->cols - filter.get_cols() + 1,
        padding ? this->depth : this->depth - filter.get_depth() + 1
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("convolution.spv"); }
    shader_exec(shader, result, filter, float_t(padding)); // OVERLOAD_3
    return result;
}

// turn rows into columns and vice versa
VkVec VkVec::transpose() const {
    VkVec result(this->cols, this->rows, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("transpose.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// matrix inversion
VkVec VkVec::inverse(uint32_t iterations) const {

    // defining a preliminary approximation matrix X
    VkVec X(this->cols, this->rows, this->depth);
    X.fill_random_uniform(-1,1);

    // decomposing 'this' into diagonal and triagonals
    VkVec U(this->cols, this->rows, this->depth);
    U = this->upper_trigonal();

    VkVec L(this->cols, this->rows, this->depth);
    L = this->lower_trigonal();

    VkVec D(this->cols, this->rows, this->depth);
    D = this->diagonal();

    // defining an identity matrix of size cols x cols
    VkVec I(this->cols, this->cols, this->depth);

    // main loop
    for (uint32_t i = 0; i < iterations; i++) {
        // TODO
    }
    
    return X;
}

// reverse sorting
VkVec VkVec::mirror(bool mirror_rows, bool mirror_cols, bool mirror_depth) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("mirror.spv"); }
    shader_exec(shader, result, int(mirror_rows), int(mirror_cols), int(mirror_depth)); // OVERLOAD_4
    return result;
}


VkVec VkVec::diagonal() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("diagonal.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

VkVec VkVec::upper_trigonal() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("upper_trigonal.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

VkVec VkVec::lower_trigonal() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lower_trigonal.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// rearrange the array elements based on a source index map (holding the flat indices, with elements of the index map within range 0 to this->elements)
VkVec VkVec::remap_to(const VkVec& index_map) const {
    VkVec result(index_map.rows, index_map.cols, index_map.depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("remap_to.spv"); }
    shader_exec(shader, result, index_map); // OVERLOAD_3
    return result;
}

// +=================================+   
// | 1d vector statistics            |
// +=================================+

// return struct for correlation results
struct VkVec::CorrelationResult {
public:

    // predict a y value for a given x element, assuming linear correlation
    float_t predict(float_t x) const { return slope * x + y_intercept; }

    // print linear correlation results to the console
    void print() const {
        std::cout
            << "=========================================================================="
            << "\nCorrelation Results (this=x vs. other=y):"
            << "\n   - mean value of x = " << x_mean
            << "\n   - mean value of y = " << y_mean
            << "\n   - variance of x = " << x_variance
            << "\n   - variance of y = " << y_variance
            << "\n   - standard deviation of x = " << x_stddev
            << "\n   - standard deviation of y = " << y_stddev
            << "\n   - regression line y-intercept = " << y_intercept
            << "\n   - regression line slope = " << slope
            << "\n   - covariance between x & y = " << covariance
            << "\n   - Pearson correlation coefficient R = " << Pearson_R
            << "\n   - Spearman correlation coefficient Rho = " << Spearman_Rho
            << "\n   - coefficient of determination (r-squared) = " << r_squared
            << "\n   - total sum of squares (SST) = " << SST
            << "\n   - explained sum of squares (SSE) = " << SSE
            << "\n   - residual sum of squares (SSR) = " << SSR
            << "\n   - mean squared error (MSE) = " << MSE
            << "\n   - mean squared regression (MSR) = " << MSR
            << "\nHypothesis Testing:"
            << "\n   - z-score = " << z_score
            << "\n   - t-score = " << t_score
            << "\n==========================================================================" << std::endl;
    }

    // constructor
    CorrelationResult(int elements) {
        y_predict = new VkVec(elements);
    }

    // destructor
    ~CorrelationResult() {
        if (y_predict != nullptr) {
            y_predict->destroy();
            y_predict = nullptr;
        }
    }

    // public variables
    VkVec* y_predict = nullptr;
    float_t x_mean, y_mean = 0;
    float_t x_variance, y_variance = 0;
    float_t x_stddev, y_stddev = 0;
    float_t y_intercept, slope = 0;
    float_t covariance = 0;
    float_t Pearson_R, Spearman_Rho = 0;
    float_t r_squared = 0;
    float_t RSS, SST, SSE, SSR, MSE, MSR = 0;
    float_t z_score, t_score = 0;
};

VkVec::CorrelationResult VkVec::correlation(const VkVec& other) const {
    CorrelationResult result(this->rows);

    if (this->dimensions != 1) {
        Log::log(WARNING, "invalid usage of method VkVec::correlation(): 'this' must be a 1d array but is ", this->dimensions, "d");
        return result;
    }

    if (other.get_dimensions() != 1) {
        Log::log(WARNING, "invalid usage of method VkVec::correlation(): 'other' must be a 1d array but is ", other.get_dimensions(), "d");
        return result;
    }

    if (this->rows != other.get_rows()) {
        Log::log(WARNING, "invalid usage of method VkVec::correlation(): 'this' has ", this->elements, " elements but 'other' has ", other.get_elements(), " elements; they must be 1d arrays of equal size");
        return result;
    }

    if (this->elements == 0 || this->rows == 0 || (this->rows == 1 && this->elements > this->rows)) {
        Log::log(WARNING, "invalid usage of method VkVec::correlation(): 'this' array is empty (i.e. row elements = 0)");
        return result;
    }

    // get core values of linear correlation
    result.x_mean = this->mean();
    result.y_mean = other.mean();
    result.covariance = (*this - result.x_mean).scalar_product(other - result.y_mean) / this->elements;
    result.x_variance = this->variance();
    result.y_variance = other.variance();
    result.x_stddev = result.x_variance != 0 ? std::sqrt(result.x_variance) : float_t(NAN);
    result.y_stddev = result.y_variance != 0 ? std::sqrt(result.y_variance) : float_t(NAN);
    result.Pearson_R = result.x_stddev * result.y_stddev != 0 ? result.covariance / (result.x_stddev * result.y_stddev) : float_t(NAN);
    result.SST = this->elements * result.x_variance;
    result.slope = result.x_variance != 0 ? result.covariance / result.x_variance : float_t(NAN);
    result.y_intercept = result.y_mean - result.slope * result.x_mean;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("linear_predict.spv"); }
    shader_exec(shader, *result.y_predict, result.y_intercept, result.slope); // OVERLOAD_2

    result.SSE = (*result.y_predict - result.y_mean).pow().sum();
    result.SSR = (other - *result.y_predict).pow().sum();
    result.r_squared = result.SST != 0 ? result.SSE / result.SST : NAN; //=SSE/SST, equal to 1-SSR/SST
    result.MSE = result.SSE / this->elements;

    // Spearman correlation, assuming non-linear monotonous dependence
    result.Spearman_Rho = 1 - 6 * ((this->rank() - other.rank()).pow()).sum() / std::fmax(this->elements * (std::pow(this->elements, 2) - 1), 1.0f);

    // test significance against null hypothesis
    float_t fisher_transform = 0.5f * std::log((1.0f + result.Spearman_Rho) / (1.0f - result.Spearman_Rho));
    result.z_score = std::sqrt(float_t(this->elements - 3) / 1.06) * fisher_transform;
    result.t_score = result.Spearman_Rho * std::sqrt(std::fmax(float_t(this->elements) - 2.0f, 1.0f) / std::fmax(1.0f - std::pow(result.Spearman_Rho, 2), 1.0f));

    return result;
}

// nested struct for polynomial regression
struct VkVec::RegressionResult {
public:
    // public variables
    float_t SSR = 0;
    float_t SST = 0;
    float_t SSE = 0;
    float_t MSE;
    float_t y_mean = 0;
    float_t x_mean = 0;
    float_t r_squared;
    VkVec* coefficients = nullptr;
    VkVec* y_predict = nullptr;
    VkVec* residuals = nullptr;

    bool is_good_fit(float_t threshold = 0.95) const { return r_squared > threshold; }

    std::string get_equation() const {
        std::string equation = "y = ";

        // add coefficients for powers > 1
        for (uint32_t p = power; p > 1; p--) {
            equation += std::to_string(coefficients->get(p)) + "x^" + std::to_string(p) + " + ";
        }

        // add slope = coefficient[1]
        equation += power > 0 ? std::to_string(coefficients->get(1)) + "x + " : "";

        // add error = y_intercept = coefficient[0]
        return equation + std::to_string(coefficients->get(0));
    }

    void print() const {
        std::cout
            << "=========================================================================="
            << "\nRegression Results (this=x vs. other=y):"
            << "\n   - equation: " << get_equation()
            << "\n   - mean value of x = " << x_mean
            << "\n   - mean value of y = " << y_mean
            << "\n   - coefficient of determination (r-squared) = " << r_squared
            << "\n   - total sum of squares (SST) = " << SST
            << "\n   - explained sum of squares (SSE) = " << SSE
            << "\n   - residual sum of squares (SSR) = " << SSR
            << "\n   - mean squared error (MSE) = " << MSE
            << "\n==========================================================================" << std::endl;
    }

    float_t predict(const float_t x) const {
        float_t result = 0;
        for (uint32_t p = 0; p <= power; p++) {
            result += coefficients->get(p) * std::pow(x, p);
        }
        return result;
    };

    // constructor & destructor
    RegressionResult(const uint32_t elements, const uint32_t power) : power(power) {
        if (coefficients == nullptr) {
            coefficients = new VkVec(power + 1);
        }
        if (y_predict == nullptr) {
            y_predict = new VkVec(elements);
        }
        if (residuals == nullptr) {
            residuals = new VkVec(elements);
        }
    };

    ~RegressionResult() {
        if (coefficients != nullptr) {
            delete coefficients;
        }
        if (y_predict != nullptr) {
            delete y_predict;
        }
        if (residuals != nullptr) {
            delete residuals;
        }
    }
private:
    uint32_t power;
};

// performs polynomial regression (to the specified power)
// with the source array as x and a second array
// as the corresponding y data;
// use power=1 for linear regression;
// make sure that both vectors are 1d and have the same number of
// elements
VkVec::RegressionResult VkVec::regression(const VkVec& other, const uint32_t power) const {

    RegressionResult result(this->elements, power);

    if (this->dimensions != 1) {
        Log::log(WARNING, "invalid usage of method VkVec::regression(): 'this' must be a 1d array but is ", this->dimensions, "d");
        return result;
    }

    if (other.get_dimensions() != 1) {
        Log::log(WARNING, "invalid usage of method VkVec::regression(): 'other' must be a 1d array but is ", other.get_dimensions(), "d");
        return result;
    }

    if (this->rows != other.get_rows()) {
        Log::log(WARNING, "invalid usage of method VkVec::regression(): 'this' has ", this->elements, " elements but 'other' has ", other.get_elements(), " elements; they must be 1d arrays of equal size");
        return result;
    }

    if (this->elements == 0 || this->rows == 0 || (this->rows == 1 && this->elements > this->rows)) {
        Log::log(WARNING, "invalid usage of method VkVec::regression(): 'this' array is empty (i.e. row elements = 0)");
        return result;
    }

    // Create 2d matrix of x values raised to different powers
    VkVec X(this->elements, power + 1);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("power_matrix.spv"); }
    shader_exec(shader, X, 0.0f); // OVERLOAD_2

    VkVec Xt = X.transpose();
    *result.coefficients = (Xt * X).inverse() * Xt * other;

    // Get R-squared value and other statistics
    result.x_mean = this->mean();
    result.y_mean = other.mean();

    result.y_predict->fill(0);
    for (uint32_t p = 0; p < power; p++) {
        *result.y_predict *= *this * result.coefficients->get(p);
    }

    result.SST = this->variance() * this->elements;
    result.SSE = (*result.y_predict - result.y_mean).pow().sum();
    result.SSR = (other - *result.y_predict).pow().sum();
    result.r_squared = result.SST != 0 ? result.SSE / result.SST : NAN; //=SSE/SST, equal to 1-SSR/SST
    result.MSE = result.SSE / this->elements;

    // calculate residuals
    *result.residuals = *this - *result.y_predict;

    return result;
}

float_t VkVec::pop_last() {
    if (this->dimensions != 1) {
        Log::log(WARNING, "invalid usage of method float_t pop_last() with ", this->dimensions, " array (must be 1d)");
        return NAN;
    }
    float result = this->get(this->rows - 1);
    this->erase_row(this->rows - 1);
    return result;
}

float_t VkVec::pop_first() {
    if (this->dimensions != 1) {
        Log::log(WARNING, "invalid usage of method float_t pop_first() with ", this->dimensions, " array (must be 1d)");
        return NAN;
    }
    float result = this->get(0);
    this->erase_row(0);
    return result;
}

VkVec VkVec::erase_row(const uint32_t row_index) {
    if (row_index >= this->rows - 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec VkVec::erase_row(const uint32_t row_index) with a row index of ",
            row_index, ": the array only has ", this->rows, " row(s); function will have no effect");
        VkVec result;
        result = *this;
        return result;
    }
    VkVec result(this->rows - 1, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_row.spv"); }
    shader_exec(shader, result, (int)row_index); // OVERLOAD_4
    return result;
}

VkVec VkVec::erase_col(const uint32_t col_index) {
    if (col_index >= this->cols - 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec VkVec::erase_col(const uint32_t col_index) with a column index of ",
            col_index, ": the array only has ", this->cols, " column(s); function will have no effect");
        VkVec result;
        result = *this;
        return result;
    }
    VkVec result(this->rows, this->cols - 1, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_col.spv"); }
    shader_exec(shader, result, (int)std::min(col_index, this->cols - 1)); // OVERLOAD_4
    return result;
}

VkVec VkVec::erase_depth(const uint32_t depth_layer_index) {
    if (depth_layer_index >= this->depth - 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec VkVec::erase_depth(const uint32_t depth_layer_index) with a depth layer index of ",
            depth_layer_index, ": the array only has ", this->depth, " layer(s); function will have no effect");
        VkVec result;
        result = *this;
        return result;
    }
    VkVec result(this->rows, this->cols, this->depth - 1);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_depth.spv"); }
    shader_exec(shader, result, (int)std::min(depth_layer_index, this->depth - 1)); // OVERLOAD_4
    return result;
}

// returns a vector that contains an ascending order ranking for each corresponding element of 'this';
// only to be used with 1d vectors
VkVec VkVec::rank() const {

    // initialize ranking
    VkVec ranking(this->rows, 1, 1);
    ranking.fill_range(0.0f, 1.0f);

    if (this->dimensions > 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec rank() const' with ", this->dimensions, "d array; this method is designed to be used only with 1d arrays");
        return ranking;
    }

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("rank.spv"); }
    shader_exec(shader, ranking, 0.0f); // OVERLOAD_2
    return ranking;
}

// performs an augmented Dickey-Fuller test
// (=unit root test for stationarity) on the sample array;
// The test returns a p-value, which is used to determine whether or not
// the null hypothesis that the dataset has a unit root
// (=implying that the sample is non-stationary and has a trend) is rejected.
// If the p-value is less than a chosen significance level (usually 0.05),
// then the null hypothesis is rejected and it is concluded that the
// time series dataset does not have a unit root and is stationary.
// The method for differencing is set to first order integer by default,
// but can be changed to other methods via the method's arguments
float_t VkVec::Dickey_Fuller() const {
    // correlate a copy of the array with a stationary transformation of itself
    VkVec copy; copy = *this;
    auto correlation_result = copy.erase_row(0).correlation(this->stationary());
    float R = correlation_result.Pearson_R;
    // calculate result
    return R * std::sqrt((float_t)(this->elements - 1) / (1 - std::pow(R, 2)));
}

// takes the source vector and another vector (passed as parameter) and
// performs an Engle-Granger test in order to test the given numeric sample
// for cointegration, i.e. checking series data for a long-term relationship.
// The test was proposed by Clive Granger and Robert Engle in 1987.
// If the returned p-value is less than a chosen significance level (typically 0.05),
// it suggests that the two time series are cointegrated and have a long-term relationship.
// Make sure that both VkVec have the same number of elements!
float_t VkVec::Engle_Granger(const VkVec& other) const {
    auto regression_result = this->stationary().regression(other.stationary());
    return regression_result.residuals->Dickey_Fuller();
}

// returns a stationary transformation of the vector data,
// using first degree differencing
// e.g. for time series data;
VkVec VkVec::stationary() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// returns a stationary transformation of the vector data,
// using first degree logreturn differencing
// e.g. for time series data;
VkVec VkVec::stationary_log() const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary_log.spv"); }
    shader_exec(shader, result, 0.0f); // OVERLOAD_2
    return result;
}

// returns a stationary transformation of the vector data,
// using fractional differencing
// e.g. for time series data;
VkVec VkVec::stationary_fract(float_t degree, float_t exponent) const {
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary_fract.spv"); }
    shader_exec(shader, result, degree, exponent); // OVERLOAD_2
    return result;
}

// ascending sorting for 1d vectors
VkVec VkVec::sort() const {
    VkVec result; result = *this;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sort.spv"); }
    shader_exec(shader, result, 0.0f);
    return result;
}

float_t VkVec::covariance(const VkVec& other) const {
    return (*this - this->mean()).scalar_product(other - other.mean()) / this->elements;
}

// +=================================+   
// | Output                          |
// +=================================+

// print the vector or array to the console
// use precision argument for decimal places (use negative number for unformatted full available precision)
void VkVec::print(std::string comment, std::string delimiter, bool with_indices, bool rows_inline, int32_t precision) const {
    uint32_t decimals = std::pow(10, precision);
    std::cout << comment;
    if (comment != "") {
        std::cout << "\n";
    }

    if (this->dimensions == 1 && rows_inline) {
        for (uint32_t x = 0; x < this->rows; x++) {
            if (with_indices) {
                std::cout << "[" << x << "]=";
            }
            std::cout << this->get(x, 0, 0);
            if (x != this->rows - 1) {
                std::cout << delimiter;
            }
        }
        std::cout << "\n" << std::flush;
    }
    else {
        for (uint32_t x = 0; x < this->rows; x++) {
            for (uint32_t y = 0; y < this->cols; y++) {
                if (this->depth == 1) {
                    if (with_indices) {
                        if (this->dimensions == 1) {
                            std::cout << "[" << x << "]=";
                        }
                        else {
                            std::cout << "[" << x << "][" << y << "]=";
                        }
                    }
                    std::cout << (precision >= 0 ? std::round(this->get(x, y, 0) * decimals) / decimals : this->get(x, y, 0));
                    // add delimiter before next column
                    if (y != this->cols - 1) {
                        std::cout << delimiter;
                    }
                }
                else {
                    std::cout << "{";
                    for (uint32_t z = 0; z < this->depth; z++) {
                        if (with_indices) {
                            std::cout << "[" << x << "][" << y << "][" << z << "]=";
                        }
                        std::cout << (precision >= 0 ? std::round(this->get(x, y, z) * decimals) / decimals : this->get(x, y, z));
                        if (z != this->depth - 1) {
                            std::cout << delimiter;
                        }
                    }
                    std::cout << "}";
                    // add space before next column
                    if (y != this->cols - 1) {
                        std::cout << " ";
                    }
                }
            }
            // add line break before next row
            std::cout << "\n";
        }
        // flush to console
        std::cout << std::flush;
    }
}

// +=================================+   
// | Protected Class Members         |
// +=================================+


// protected helper method for descriptor pool destruction
void VkVec::destroy_descriptor_pool() {
    if (descriptor_pool != nullptr) {
        delete descriptor_pool;
        descriptor_pool = nullptr;
    }
}

uint32_t VkVec::flat_index(uint32_t row, uint32_t col, uint32_t depth_layer) const {
    return  row * (this->cols * this->depth) +
            col * this->cols +
            depth_layer;
}


// +=================================+   
// | Shader Execution                |
// +=================================+

// shader execution OVERLOAD_1
void VkVec::shader_exec(ShaderModule& shader, float_t& result, const float_t& constant1, const float_t& constant2, const float_t& constant3) const {
    static Buffer<float_t> result_buffer(manager->get_device(), BufferUsage::STORAGE, 1);
    result_buffer.set(result, 0); // initialize to value of 'float_t& result' argument
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, true, this->elements, 1, 1, 32);

    result = result_buffer.get(0);

    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_2
void VkVec::shader_exec(ShaderModule& shader, VkVec& result, const float_t& constant1, const float_t& constant2, const float_t& constant3) const {
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.get_rows(), result.get_cols(), result.get_depth() });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.get_elements(), 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_3
void VkVec::shader_exec(ShaderModule& shader, VkVec& result, const VkVec& other, const float_t& constant1, const float_t& constant2, const float_t& constant3) const {
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*other.data_buffer, 1);
    descriptor_set.bind_buffer(*result.data_buffer, 2);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ other.get_rows(), other.get_cols(), other.get_depth() });
    push_constants.add_values({ result.get_rows(), result.get_cols(), result.get_depth() });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.get_elements(), 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_4
void VkVec::shader_exec(ShaderModule& shader, VkVec& result, const int32_t& constant1, const int32_t& constant2, const int32_t& constant3) const {
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.get_rows(), result.get_cols(), result.get_depth() });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.get_elements(), 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_5
void VkVec::shader_exec(ShaderModule& shader, int32_t& result, const float_t& constant1, const float_t& constant2, const float_t& constant3) const {
    static Buffer<int32_t> result_buffer(manager->get_device(), BufferUsage::STORAGE, 1);
    result_buffer.set(result, 0); // initialize to value of 'uint32_t& result' argument

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 32);

    result = result_buffer.get(0);
    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_6
void VkVec::shader_exec(ShaderModule& shader) {

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_7
void VkVec::shader_exec(ShaderModule& shader, int32_t& result, const VkVec& other, const float& constant1, const float& constant2, const float& constant3) const {
    static Buffer<int32_t> result_buffer(manager->get_device(), BufferUsage::STORAGE, 1);
    result_buffer.set(result, 0); // initialize to value of 'uint32_t& result' argument
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result_buffer, 1);
    descriptor_set.bind_buffer(*other.data_buffer, 2);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ other.get_rows(), other.get_cols(), other.get_depth() });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, 32);

    result = result_buffer.get(0);
    pipeline.destroy();
    descriptor_set.free();
}

// shader execution OVERLOAD_8
void VkVec::shader_exec(ShaderModule& shader, VkVec& result, const VkVec& other1, const VkVec& other2, const float& constant1, const float& constant2, const float& constant3) const {
    static Buffer<int32_t> result_buffer(manager->get_device(), BufferUsage::STORAGE, 1);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result_buffer, 1);
    descriptor_set.bind_buffer(*other1.data_buffer, 2);
    descriptor_set.bind_buffer(*other2.data_buffer, 3);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.get_rows(), result.get_cols(), result.get_depth() });
    push_constants.add_values({ other1.get_rows(), other1.get_cols(), other1.get_depth() });
    push_constants.add_values({ other2.get_rows(), other2.get_cols(), other2.get_depth() });
    push_constants.add_values({ constant1, constant2, constant3 });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, 32);

    pipeline.destroy();
    descriptor_set.free();
}
#endif