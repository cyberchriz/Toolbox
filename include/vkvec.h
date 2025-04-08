// author and copyright: Christian Suer (cyberchriz)
// description: class for parallel floating point data structure computations on the GPU (using Vulkan)

#ifndef VKVEC_H
#define VKVEC_H

#pragma once
#define NOMINMAX
#include "angular.h"
#include "log.h"
#include "vkcontext.h"
#include <__msvc_ostream.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <minwindef.h>
#include <seed.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <vulkan_core.h>
#include <Windows.h>

// forward declarations
class VkVec;
typedef VkVec vec;

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

    void set(const float_t value, const uint32_t row_index, const uint32_t col_index = 0, const uint32_t layer_index = 0);
    void set(std::vector<float_t>& data);
    float_t get(const uint32_t row_index, const uint32_t col_index = 0, const uint32_t layer_index = 0) const;
    std::vector<float> get() const;
    Buffer<float_t>* get_data_buffer() const;
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
    void fill_index();

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
    float_t norm() const;

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

    VkVec ident() const;                                VkVec ident_drv() const;
    VkVec sigmoid() const;                              VkVec sigmoid_drv() const;
    VkVec elu(float_t alpha = 0.01) const;              VkVec elu_drv(float_t alpha = 0.01) const;
    VkVec relu(float_t alpha = 0.01) const;             VkVec relu_drv(float_t alpha = 0.01) const;
                                                        VkVec tanh_drv(AngularMeasure unit = RAD) const;

    // +=================================+   
    // | Outlier Treatment               |
    // +=================================+
    
    VkVec outliers_truncate(float_t z_score = 3.0f) const;
    VkVec outliers_truncate(const float_t min_value, const float_t max_value) const;
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
    VkVec erase_layer(const uint32_t depth_layer_index);
    VkVec add_rows(const int32_t rows = 1, float_t init_value = 0.0f) const;
    VkVec add_cols(const int32_t cols = 1, float_t init_value = 0.0f) const;
    VkVec add_depth(const int32_t layers = 1, float_t init_value = 0.0f) const;
    VkVec resize(const uint32_t rows, const uint32_t cols = 1, const uint32_t depth = 1, float_t init_value = 0.0f) const;
    VkVec concatenate(const VkVec& other, const uint32_t axis = 0) const;
    VkVec padding(const float_t value = 0.0f, const uint32_t before_rows = 1, const uint32_t after_rows = 1, const uint32_t before_cols = 0, const uint32_t after_cols = 0, const uint32_t above_layers = 0, const uint32_t below_layers = 0) const;
    VkVec stationary() const;
    VkVec stationary_log() const;
    VkVec stationary_fract(float_t degree = 1.0f, float_t exponent = 1.0f) const;
    VkVec sort() const;
    VkVec pool_max(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_maxabs(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_min(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec pool_mean(const int32_t slider_rows = 2, const int32_t slider_cols = 1, const int32_t slider_depth = 1) const;
    VkVec convolution(const VkVec& kernel, bool padding = false) const;
    VkVec transpose() const;
    VkVec inverse(const float_t tolerance = 0.00001f, const uint32_t max_iterations = 20) const;
    VkVec mirror(bool mirror_rows = true, bool mirror_cols = true, bool mirror_depth = true) const;
    VkVec diagonal() const;
    VkVec upper_trigonal() const;
    VkVec lower_trigonal() const;
    VkVec remap(const VkVec& source, const VkVec& target, const VkVec& target_index_map_on_source) const;
    VkVec remap(const VkVec& target_index_map_on_source) const;

    // +=================================+   
    // | 1d vector statistics            |
    // +=================================+

    struct CorrelationResult;
    struct RegressionResult;

    CorrelationResult correlation(const VkVec& other) const;
    RegressionResult VkVec::regression(const VkVec& other, const uint32_t power = 1) const;
    float_t Dickey_Fuller() const;
    float_t Engle_Granger(const VkVec& other) const;
    float_t covariance(const VkVec& other) const;

    // +=================================+   
    // | Output                          |
    // +=================================+
    
    void print(std::string comment = "", std::string delimiter = "|", bool with_indices = false, bool rows_inline = false, int32_t precision = 3) const;

    static VkManager* manager;
    static DescriptorPool* descriptor_pool;
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
    
    void destroy();

    static void destroy_descriptor_pool();

    uint32_t flat_index(uint32_t row, uint32_t col = 0, uint32_t depth_layer = 0) const;

    void copy_resources(const VkVec& other);

};














// DEFINITIONS
// ===============================================================================================================================

// +=================================+   
// | Static Member Initializations   |
// +=================================+

DescriptorPool* VkVec::descriptor_pool = nullptr;
VkManager* VkVec::manager = nullptr;


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
        std::vector<const char*> instance_layer_names = {};
        #ifdef _DEBUG
        instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
        #endif		

        // enable instance extensions
        std::vector<const char*> instance_extension_names;
        // TODO: push_back required instance extensions here

        // enable device extensions
        std::vector<const char*> device_extension_names = {
            "VK_EXT_descriptor_indexing",
            "VK_EXT_shader_atomic_float",
            "VK_KHR_storage_buffer_storage_class",
            "VK_EXT_shader_image_atomic_int64",
            "VK_KHR_shader_non_semantic_info"
        };

        manager = VkManager::make_singleton(
            instance_layer_names,
            instance_extension_names,
            device_extension_names
        );
    }
    else {
        manager = VkManager::get_singleton();
    }

    if (descriptor_pool == nullptr) {
        static constexpr uint32_t max_sets_within_pool = 1;
        descriptor_pool = new DescriptorPool(manager->get_device(), max_sets_within_pool);
        std::atexit(&VkVec::destroy_descriptor_pool);
    }

    // add a command buffer + data buffer
    command_buffer = new CommandBuffer(manager->get_device(), QueueFamily::COMPUTE, manager->get_command_pool_compute());
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
void VkVec::set(const float_t value, const uint32_t row_index, const uint32_t col_index, const uint32_t layer_index) {
    // using flat index as 'row' index
    this->data_buffer->set(value, row_index * this->cols * this->depth + col_index * this->depth + layer_index);
}

// copies raw data from a std::vector<float_t> to the data buffer
// of the underlying VkVec array
void VkVec::set(std::vector<float_t>& data) {
    data_buffer->write(data);
}

// returns the value of an array element via its flattened index
float_t VkVec::get(const uint32_t row_index, const uint32_t col_index, const uint32_t layer_index) const {
    // using flat index as 'row' index
    return data_buffer->get(row_index * this->cols * this->depth + col_index * this->depth + layer_index);
}

// returns a flat copy of the raw data of the underlying buffer as type std::vector<float_t>
std::vector<float> VkVec::get() const {
    return data_buffer->read();
}

// returns the buffer containg the raw array data
Buffer<float_t>* VkVec::get_data_buffer() const {
    return this->data_buffer;
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
    result += "}";
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

    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_row.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(row_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// returns a single column sliced from a 2d or 3d array
VkVec VkVec::get_col(int32_t col_index) const {
    VkVec result(this->rows, 1, this->depth);

    if (col_index >= this->cols || col_index < 0) {
        Log::log(ERROR, "invalid usage of method 'VkVec get_col(uint32_t col_index)' with invalid column index; index is ", col_index, ", the underlying array has ", this->cols, " column(s)");
    }

    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_col.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(col_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// returns a single depth layer sliced from a 3d array
VkVec VkVec::get_layer(int32_t layer_index) const {
    VkVec result(this->rows, this->cols, 1);

    if (layer_index >= this->depth || layer_index < 0) {
        Log::log(ERROR, "invalid usage of method 'VkVec get_layer(uint32_t layer_index)' with invalid depth layer index; index is ", layer_index, ", the underlying array has ", this->depth, " depth layer(s)");
    }

    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("get_layer.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(layer_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// +=================================+   
// | Fill, Initialize                |
// +=================================+

// fill entire array with given floating point value
void VkVec::fill(const float_t value) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// initialize the entire array with zeros
void VkVec::fill_zero() {
    this->fill(0.0f);
}

// fill entire array with identity matrix
void VkVec::fill_identity() {
    static constexpr uint32_t workgroup_size = 256;
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
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with values from a random normal (=gaussian) distribution
void VkVec::fill_random_gaussian(const float_t mu, const float_t sigma) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_gaussian.spv"); }
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values(seed32());
    push_constants.add_values(mu);
    push_constants.add_values(sigma);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with values from a random uniform distribution
void VkVec::fill_random_uniform(const float_t min, const float_t max) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_uniform.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(min);
    push_constants.add_values(max);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with values from a random uniform distribution
void VkVec::fill_random_uniform_int(const int32_t min, const int32_t max) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_random_uniform_int.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(min);
    push_constants.add_values(max);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// randomly sets the specified fraction of the values to zero and the rest to 1 (default: 0.5, i.e. 50%)
void VkVec::fill_random_binary(float_t ratio) {
    static constexpr uint32_t workgroup_size = 256;
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
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(valid_ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// randomly sets the specified fraction of the values to -1 and the rest to +1 (default: 0.5, i.e. 50%)
void VkVec::fill_random_sign(float_t ratio) {
    static constexpr uint32_t workgroup_size = 256;
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
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(valid_ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}


// fills the array with a continuous
// range of numbers (with specified start parameter
// referring to the zero position and a step parameter)
// in all dimensions
void VkVec::fill_range(const float_t start, const float_t step) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_range.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->rows);
    push_constants.add_values(this->cols);
    push_constants.add_values(this->depth);
    push_constants.add_values(start);
    push_constants.add_values(step);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

void VkVec::fill_dropout(float_t ratio) {
    static constexpr uint32_t workgroup_size = 256;
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
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(ratio);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with normal "Xavier" weight initialization
// (by Xavier Glorot & Bengio) for tanh activation
void VkVec::fill_Xavier_normal(uint32_t fan_in, uint32_t fan_out) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_normal.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(fan_in);
    push_constants.add_values(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with uniform "Xavier" weight initializiation
// (by Xavier Glorot & Bengio), e.g. for tanh activation
void VkVec::fill_Xavier_uniform(uint32_t fan_in, uint32_t fan_out) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_uniform.spv"); }

    float_t seed = (float_t)(double(rand()) / RAND_MAX);

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed);
    push_constants.add_values(fan_in);
    push_constants.add_values(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with uniform "Xavier" weight initialization
// for sigmoid activation
void VkVec::fill_Xavier_sigmoid(uint32_t fan_in, uint32_t fan_out) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_Xavier_sigmoid.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(fan_in);
    push_constants.add_values(fan_out);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with "Kaiming He" normal weight initialization,
// used for ReLU activation
void VkVec::fill_He_ReLU(uint32_t fan_in) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_He_ReLU.spv"); }

    static std::vector<DescriptorType> types = {
    STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(fan_in);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fill with modified "Kaiming He" nornal weight initialization,
// used for ELU activation
void VkVec::fill_He_ELU(uint32_t fan_in) {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_He_ELU.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());
    push_constants.add_values(fan_in);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// fills the array elements with their flat indices
void VkVec::fill_index() {
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("fill_index.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
}

// +=================================+   
// | Distribution Properties         |
// +=================================+

// returns the lowest value of the VkVec,
// across all dimensions
float_t VkVec::min() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min.spv"); }
    
    Buffer<float> result(manager->get_device(), STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), STORAGE, workgroups);
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types);
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    
    pipeline.destroy();
    descriptor_set.destroy();
    
    return result.get(0);
}

// returns the highest value of the VkVec,
// across all dimensions
float_t VkVec::max() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max.spv"); }

    Buffer<float> result(manager->get_device(), STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types);
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result.get(0);
}

// returns the value of the VkVec with the highest
// deviation from zero, across all dimensions
float_t VkVec::maxabs() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("maxabs.spv"); }

    Buffer<float> result(manager->get_device(), STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types);
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    // std::cout << "expected variance result: " << (this->operator-((this->operator/(elements)).sum())).pow().operator/(elements - 1).sum() << std::endl;
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("variance.spv"); }

    Buffer<float> partial_sum(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev2(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, 1);
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(partial_sum, 2);
    descriptor_set.bind_buffer(mdev2, 3);
    descriptor_set.bind_buffer(signal, 4);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    
    return result.get(0);
}

// returns the standard deviation of all values a the vector, matrix or array
float_t VkVec::stddev() const {
    return std::sqrt(this->variance());
}

// returns the skewness of all data of the VkVec
// across all dimensions
float_t VkVec::skewness() const {
    // TODO: check this code and its shader again; result isn't correct
    VkVec mdev = this->operator-(this->mean());
    //std::cout << "expected result = " << mdev.pow(2).sum()/elements / std::pow(mdev.pow(3).sum()/elements, 1.5) << std:: endl;

    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("skewness.spv"); }

    Buffer<float> partial_sum(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev2(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev3(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, 1);
    Buffer<uint32_t> signal(manager->get_device(), STORAGE, workgroups);
    
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, descriptor_types(STORAGE_BUFFER, 7)); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(partial_sum, 1);
    descriptor_set.bind_buffer(mdev2, 2);
    descriptor_set.bind_buffer(mdev3, 3);
    descriptor_set.bind_buffer(result, 4);
    descriptor_set.bind_buffer(signal, 5);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result.get(0);
}

// returns the kurtosis of all data of the VkVec
// across all dimensions
float_t VkVec::kurtosis() const {
    // TODO: check this code and its shader again; result isn't correct
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("kurtosis.spv"); }

    Buffer<float> partial_sum(manager->get_device(), BufferUsage::STORAGE, workgroups);
    Buffer<float> mdev2(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> mdev4(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, 1);
    Buffer<uint32_t> signal(manager->get_device(), STORAGE, workgroups);
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(partial_sum, 1);
    descriptor_set.bind_buffer(mdev2, 2);
    descriptor_set.bind_buffer(mdev4, 3);
    descriptor_set.bind_buffer(result, 4);
    descriptor_set.bind_buffer(signal, 5);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result.get(0);
}

// returns the Euklidean norm (=distance from origin)
float VkVec::norm() const {
    return this->pow(2).sum();
}

// +=================================+   
// | Addition                        |
// +=================================+

// returns the sum of all array elements;
float_t VkVec::sum() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sum.spv"); }
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);
    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result.get(0);
}

// elementwise addition of the specified value to all values of the array
VkVec VkVec::operator+(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_plus_value.spv"); }
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };
    
    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// returns the resulting array of the elementwise addition of two arrays
VkVec VkVec::operator+(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_plus_other.spv"); }
    
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
    push_constants.add_values({this->rows, this->cols, this->depth});
    push_constants.add_values({other.rows, other.cols, other.depth});

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_minus_other.spv"); }

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
    push_constants.add_values({ other.rows, other.cols, other.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
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
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("product.spv"); }

    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result.get(0);
}

// elementwise multiplication with a scalar
VkVec VkVec::operator*(const float_t factor) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_multiply_factor.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
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

// Alias for 2D or 3D matrix multiplication;
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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, other.get_cols(), this->depth);
        
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("matrix_product.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.get_elements(), 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// elementwise multiplication of the values of the current
// array with the corresponding values of a second array,
// resulting in the 'Hadamard product';
// the dimensions of the two arrays must match!
// if they don't: only the common elements will be part of the result array
VkVec VkVec::Hadamard_product(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("hadamard_product.spv"); }
    
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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    
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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("hadamard_division.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    static constexpr uint32_t workgroup_size = 256;

    if (value == 0) {
        Log::log(WARNING,
            "invalid usage of method 'VkVec VkVec::operator%(const float_t value) const' ",
            "with value=0 (zero division is undefined) --> 'this' will remain unmodified");
        return *this;
    }
    
    VkVec result(this->rows, this->cols, this->depth);
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("operator_modulo_value.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// +=================================+   
// | Exponentiation & Logarithm      |
// +=================================+

// elementwise exponentiation to the power of
// the specified exponent
VkVec VkVec::pow(const float_t exponent) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);
    
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pow.spv"); }
    
    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(exponent);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(std::min(this->rows, other.get_rows()), std::min(this->cols, other.get_cols()), std::min(this->depth, other.get_depth()));

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pow_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// converts the individual values of the array
// elementwise to their square root
VkVec VkVec::sqrt() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sqrt.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::log(float_t base) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("log.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(base);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::exp() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("exp.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// +=================================+   
// | Rounding                        |
// +=================================+

// rounds the values of the array elementwise
// to their nearest integers
VkVec VkVec::round() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("round.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// rounds the values of the array elementwise
// to their next lower integers
VkVec VkVec::floor() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("floor.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// returns a copy of the array that stores the values as rounded
// to their next higher integers
VkVec VkVec::ceil() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("ceil.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// returns a copy of the array that stores the
// absolute values of the source array
VkVec VkVec::abs() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("abs.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// +=================================+   
// | Min, Max                        |
// +=================================+

// elementwise minimum of the specified value
// and the data elements of the array
VkVec VkVec::min(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min_value.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise maximum of the specified value
// and the data elements of the array
VkVec VkVec::max(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max_value.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// returns the result of elementwise min() comparison
// of 'this' vs 'other'
VkVec VkVec::min(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("min_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// returns the result of elementwise max() comparison
// of 'this' vs 'other'
VkVec VkVec::max(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("max_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// +=================================+   
// | Trigonometric Functions         |
// +=================================+

// elementwise application of the cos() function
VkVec VkVec::cos(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("cos.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the sin() function
VkVec VkVec::sin(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sin.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the tan function
VkVec VkVec::tan(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tan.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the acos() function
VkVec VkVec::acos(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("acos.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the asin() function
VkVec VkVec::asin(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("asin.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the atan function
VkVec VkVec::atan(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("atan.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the hyperbolic cosine function
VkVec VkVec::cosh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("cosh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise applicatiohn of the hyperbolic sine function
VkVec VkVec::sinh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sinh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the hyperbolic tangent function
VkVec VkVec::tanh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tanh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the hyperbolic arc cosine function
VkVec VkVec::acosh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("acosh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the hyperbolic arc sine function
VkVec VkVec::asinh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("asinh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// elementwise application of the hyperbolic arc tangent function
VkVec VkVec::atanh(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("atanh.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(old_value);
    push_constants.add_values(new_value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// replaces all elements of 'this' with the corresponding element of the
// 'replacing_map' if the corresponding element of the condition map is !=0
VkVec VkVec::replace_if(const VkVec& condition_map, const VkVec& replacing_map) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace_if_other.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*condition_map.data_buffer, 1);
    descriptor_set.bind_buffer(*replacing_map.data_buffer, 2);
    descriptor_set.bind_buffer(*result.data_buffer, 3);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth} );
    push_constants.add_values({ condition_map.rows, condition_map.cols, condition_map.depth });
    push_constants.add_values({ replacing_map.rows, replacing_map.cols, replacing_map.depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// replaces all elements of 'this' with the corresponding element of the
// 'replacing_map' if the corresponding element of the condition map is !=0
VkVec VkVec::replace_if(const VkVec& condition_map, const float_t replacing_value) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("replace_if_value.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*condition_map.data_buffer, 1);
    descriptor_set.bind_buffer(*result.data_buffer, 2);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ condition_map.rows, condition_map.cols, condition_map.depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(replacing_value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// returns the number of occurrences of the specified value;
// in order to mitigate floating point number rounding imprecisions,
// this method will consider any values that differ by no more than
// epsilon = 0.0000001 from the old value as a match
uint32_t VkVec::find(const float_t& value) const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("find.spv"); }

    Buffer<float> result(manager->get_device(), BufferUsage::STORAGE, this->elements);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(result, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result.get(0);
}

// returns a VkVec array of equal dimensions as the source,
// with -1 for all corresponding negative values and +1 for all corresponding positive values
// (0 for all zeros)
VkVec VkVec::sign() const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sign.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// +=================================+   
// | Scaling                         |
// +=================================+

// scale to specified range
VkVec VkVec::scale_minmax(float_t range_from, float_t range_to) const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("scale_minmax.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(std::min(range_from, range_to));
    push_constants.add_values(std::max(range_from, range_to));

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// mean normalization scaling, i.e.
// (x - mean) / (max - min)
VkVec VkVec::scale_mean() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("scale_mean.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// scaling to zero mean and unit-variance, i.e.
// (x - mean) / sigma
VkVec VkVec::scale_standardized() const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("scale_standardized.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
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
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sigmoid.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// sigmoid activation derivative
// exp(x)/pow(exp(x)+1,2)
VkVec VkVec::sigmoid_drv() const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sigmoid.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// ELU activation function
VkVec VkVec::elu(float_t alpha) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("elu.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(alpha);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// ELU activation derivative;
// chose alpha=0 for true ELU function;
// small alpha value like e.g. 0.01 for 'leaky' ELU
VkVec VkVec::elu_drv(float_t alpha) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("elu_drv.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(alpha);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}


// ReLU activation function;
// chose alpha=0 for true ReLU function;
// small alpha value like e.g. 0.01 for 'leaky' ReLU
VkVec VkVec::relu(float_t alpha) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("relu.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(alpha);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// ReLU activation derivative;
// chose alpha=0 for true ReLU function;
// small alpha value like e.g. 0.01 for 'leaky' ReLU
VkVec VkVec::relu_drv(float_t alpha) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("relu_drv.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(alpha);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// tanh activation derivative
VkVec VkVec::tanh_drv(AngularMeasure unit) const {
    float_t factor = angle(1.0f, unit, RAD, false); // conversion factor from source unit to radians
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("tanh_drv.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(factor);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// +=================================+   
// | Outlier Treatment               |
// +=================================+

// returns a copy of the data array
// limited to the range from min_value to max_value
VkVec VkVec::outliers_truncate(const float_t min_value, const float_t max_value) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("outliers_minmax.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(std::min(min_value, max_value));
    push_constants.add_values(std::max(min_value, max_value));

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// truncate outliers by z-score mean deviation
VkVec VkVec::outliers_truncate(float_t z_score) const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("outliers_truncate.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(z_score);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// set outliers (by z-score) to mean
VkVec VkVec::outliers_mean_imputation(float_t z_score) const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("outliers_mean_imputation.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(z_score);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// set outliers (by z-score) to value
VkVec VkVec::outliers_value_imputation(float_t value, float_t z_score) const {
    static constexpr uint32_t workgroup_size = 256;
    const uint32_t workgroups = this->elements / workgroup_size + 1;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("outliers_value_imputation.spv"); }

    VkVec result(this->rows, this->cols, this->depth);
    Buffer<uint32_t> signal(manager->get_device(), BufferUsage::STORAGE, workgroups);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);
    descriptor_set.bind_buffer(signal, 2);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(z_score);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// recover -inf, +inf or nan values
VkVec VkVec::recover() const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("recover.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(seed32());

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
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
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greater_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}


VkVec VkVec::operator>=(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greaterequals_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::operator==(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("equals_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::operator!=(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("notequals_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::operator<(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("less_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::operator<=(const float_t value) const {
    static constexpr uint32_t workgroup_size = 256;

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lessequals_value.spv"); }

    VkVec result(this->rows, this->cols, this->depth);

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// elementwise comparison with second VkVec
VkVec VkVec::operator>(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greater_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::operator>=(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("greaterequals_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::operator==(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("equals_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::operator!=(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("notequals_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}
VkVec VkVec::operator<(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("less_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::operator<=(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lessequals_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("and_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::operator||(const VkVec& other) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->rows, this->cols, this->depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("or_other.spv"); }

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

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// +=================================+   
// | dynamic handling                |
// +=================================+

// conversion from 2d or 3d array to 1d vector
VkVec VkVec::flatten() const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(this->elements, 1, 1);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("flatten.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values(this->elements);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

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
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(rows, cols, depth);

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("resize.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(init_value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// stitch two VkVec arrays together along the specified axis
VkVec VkVec::concatenate(const VkVec& other, const uint32_t axis) const {
    static constexpr uint32_t workgroup_size = 256;
    if (axis > 2) {
        Log::log(ERROR, "in method VkVec::concatenate() invalid axis argument (axis is ", axis, " but no values > 2 are allowed)");
    }
    VkVec result(
        axis == 0 ? this->rows + other.get_rows() : this->rows,
        axis == 1 ? this->cols + other.get_cols() : this->cols,
        axis == 2 ? this->depth + other.get_depth() : this->depth
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("concatenate.spv"); }

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
    push_constants.add_values({ other.rows, other.cols, other.depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

// padding around vector/matrix/array using the passed value 
VkVec VkVec::padding(const float_t value, const uint32_t before_rows, const uint32_t after_rows, const uint32_t before_cols, const uint32_t after_cols, const uint32_t above_layers, const uint32_t below_layers) const {
    static constexpr uint32_t workgroup_size = 256;

    VkVec result(
        before_rows + this->rows + after_rows,
        before_cols + this->cols + after_cols,
        above_layers + this->depth + below_layers
    );

    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("padding.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ before_rows, before_cols, above_layers });
    push_constants.add_values(value);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::pool_max(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(
        std::max(int(std::ceil(float_t(this->rows) / slider_rows)), 1),
        std::max(int(std::ceil(float_t(this->cols) / slider_cols)), 1),
        std::max(int(std::ceil(float_t(this->depth) / slider_depth)), 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_max.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ slider_rows, slider_cols, slider_depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);

    pipeline.destroy();
    descriptor_set.destroy();

    return result;
}

VkVec VkVec::pool_maxabs(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(
        std::max(int(std::ceil(float_t(this->rows) / slider_rows)), 1),
        std::max(int(std::ceil(float_t(this->cols) / slider_cols)), 1),
        std::max(int(std::ceil(float_t(this->depth) / slider_depth)), 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_maxabs.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ slider_rows, slider_cols, slider_depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::pool_min(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(
        std::max(int(std::ceil(float_t(this->rows) / slider_rows)), 1),
        std::max(int(std::ceil(float_t(this->cols) / slider_cols)), 1),
        std::max(int(std::ceil(float_t(this->depth) / slider_depth)), 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_min.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ slider_rows, slider_cols, slider_depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::pool_mean(const int32_t slider_rows, const int32_t slider_cols, const int32_t slider_depth) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(
        std::max(int(std::ceil(float_t(this->rows) / slider_rows)), 1),
        std::max(int(std::ceil(float_t(this->cols) / slider_cols)), 1),
        std::max(int(std::ceil(float_t(this->depth) / slider_depth)), 1)
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("pool_mean.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ slider_rows, slider_cols, slider_depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::convolution(const VkVec& kernel, bool padding) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(
        padding ? this->rows : this->rows - kernel.get_rows() + 1,
        padding ? this->cols : this->cols - kernel.get_cols() + 1,
        padding ? this->depth : this->depth - kernel.get_depth() + 1
    );
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("convolution.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*kernel.data_buffer, 1);
    descriptor_set.bind_buffer(*result.data_buffer, 2);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ kernel.rows, kernel.cols, kernel.depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// turn rows into columns and vice versa
VkVec VkVec::transpose() const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(this->cols, this->rows, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("transpose.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// matrix inversion (via iterative approximation)
VkVec VkVec::inverse(const float_t tolerance,  const uint32_t max_iterations) const {
    VkVec X(this->cols, this->rows, this->depth); X.fill_random_uniform(-1.0f, 1.0f); // = initial guess, to be iteratively refined
    VkVec X_new(this->cols, this->rows, this->depth);
    VkVec I(this->rows, this->cols, this->depth); I.fill_identity();
    VkVec I2 = I * 2;
    for (uint32_t i = 0; i < max_iterations; i++) {
        X_new = X * (I2 - (*this) * X);
        X_new.print("X_new:");
        if (std::fabs((X - X_new).norm()) <= tolerance) {
            return X_new;
        }
        else {
            X = X_new;
        }
    }
    return X;
}

// reverse sorting
VkVec VkVec::mirror(bool mirror_rows, bool mirror_cols, bool mirror_depth) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("mirror.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values({ int(mirror_rows), int(mirror_cols), int(mirror_depth) });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}


VkVec VkVec::diagonal() const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("diagonal.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::upper_trigonal() const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("upper_trigonal.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::lower_trigonal() const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(this->rows, this->cols, this->depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("lower_trigonal.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// rearrange the source array elements based on a target index map (holding the flat indices)
VkVec VkVec::remap(const VkVec& source, const VkVec& target, const VkVec& target_index_map_on_source) const {
    static constexpr uint32_t workgroup_size = 256;
    VkVec result(target.rows, target.cols, target.depth);
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("remap.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*source.data_buffer, 0);
    descriptor_set.bind_buffer(*target.data_buffer, 1);
    descriptor_set.bind_buffer(*target_index_map_on_source.data_buffer, 2);
    descriptor_set.bind_buffer(*result.data_buffer, 3);

    PushConstants push_constants;
    push_constants.add_values({ source.rows, source.cols, source.depth });
    push_constants.add_values({ target_index_map_on_source.rows, target_index_map_on_source.cols, target_index_map_on_source.depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, source.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::remap(const VkVec& target_index_map_on_source) const {
    return remap(*this, *this, target_index_map_on_source);
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
            << "\n   - coefficient of determination (r-squared) = " << r_squared
            << "\n   - total sum of squares (SST) = " << SST
            << "\n   - explained sum of squares (SSE) = " << SSE
            << "\n   - residual sum of squares (SSR) = " << SSR
            << "\n   - mean squared error (MSE) = " << MSE
            << "\n   - mean squared regression (MSR) = " << MSR
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
    float_t x_mean = 0;
    float_t y_mean = 0;
    float_t x_variance = 0;
    float_t y_variance = 0;
    float_t x_stddev = 0;
    float_t y_stddev = 0;
    float_t y_intercept = 0;
    float_t slope = 0;
    float_t covariance = 0;
    float_t Pearson_R = 0;
    float_t r_squared = 0;
    float_t RSS = 0;
    float_t SST = 0;
    float_t SSE = 0;
    float_t SSR = 0;
    float_t MSE = 0;
    float_t MSR = 0;
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

    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("linear_predict.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.y_predict->data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.y_predict->rows, result.y_predict->cols, result.y_predict->depth });
    push_constants.add_values(result.y_intercept);
    push_constants.add_values(result.slope);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.y_predict->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();

    result.SSE = (*result.y_predict - result.y_mean).pow().sum();
    result.SSR = (other - *result.y_predict).pow().sum();
    result.r_squared = result.SST != 0 ? result.SSE / result.SST : NAN; //=SSE/SST, equal to 1-SSR/SST
    result.MSE = result.SSE / this->elements;

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
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("power_matrix.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*X.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ X.rows, X.cols, X.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();

    VkVec Xt = X.transpose();
    *result.coefficients = ((Xt * X).inverse() * Xt) * other;

    // Get R-squared value and other statistics
    result.x_mean = this->mean();
    result.y_mean = other.mean();

    result.y_predict->fill(0);
    for (uint32_t p = 0; p < power; p++) {
        *result.y_predict += this->pow(p) * result.coefficients->get(p);
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
    *this = this->erase_row(this->rows - 1);
    return result;
}

float_t VkVec::pop_first() {
    if (this->dimensions != 1) {
        Log::log(WARNING, "invalid usage of method float_t pop_first() with ", this->dimensions, " array (must be 1d)");
        return NAN;
    }
    float result = this->get(0);
    *this = this->erase_row(0);
    return result;
}

VkVec VkVec::erase_row(const uint32_t row_index) {
    if (row_index >= this->rows - 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec VkVec::erase_row(const uint32_t row_index) with a row index of ",
            row_index, ": the array only has ", this->rows, " row(s); function will have no effect");
        return *this;
    }
    VkVec result(this->rows - 1, this->cols, this->depth);
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_row.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(row_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
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
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_col.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(col_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

VkVec VkVec::erase_layer(const uint32_t depth_layer_index) {
    if (depth_layer_index >= this->depth - 1) {
        Log::log(WARNING, "invalid usage of method 'VkVec VkVec::erase_depth(const uint32_t depth_layer_index) with a depth layer index of ",
            depth_layer_index, ": the array only has ", this->depth, " layer(s); function will have no effect");
        VkVec result;
        result = *this;
        return result;
    }
    VkVec result(this->rows, this->cols, this->depth - 1);
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("erase_depth.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(depth_layer_index);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
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
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// returns a stationary transformation of the vector data,
// using first degree logreturn differencing
// e.g. for time series data;
VkVec VkVec::stationary_log() const {
    VkVec result(this->rows, this->cols, this->depth);
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary_log.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// returns a stationary transformation of the vector data,
// using fractional differencing
// e.g. for time series data;
VkVec VkVec::stationary_fract(float_t degree, float_t exponent) const {
    VkVec result(this->rows, this->cols, this->depth);
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("stationary_fract.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });
    push_constants.add_values(degree);
    push_constants.add_values(exponent);

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, result.elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
    return result;
}

// ascending sorting for 1d vectors
VkVec VkVec::sort() const {
    VkVec result; result = *this;
    static constexpr uint32_t workgroup_size = 256;
    static ShaderModule shader(manager->get_device());
    if (!shader.get()) { shader.read_from_file("sort.spv"); }

    static std::vector<DescriptorType> types = {
        STORAGE_BUFFER,
        STORAGE_BUFFER
    };

    DescriptorSet descriptor_set(manager->get_device(), *this->descriptor_pool, types); // set 0
    descriptor_set.bind_buffer(*this->data_buffer, 0);
    descriptor_set.bind_buffer(*result.data_buffer, 1);

    PushConstants push_constants;
    push_constants.add_values({ this->rows, this->cols, this->depth });
    push_constants.add_values({ result.rows, result.cols, result.depth });

    ComputePipeline pipeline(manager->get_device(), shader, push_constants, descriptor_set);
    command_buffer->compute(pipeline, this->elements, 1, 1, workgroup_size);
    pipeline.destroy();
    descriptor_set.destroy();
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



#endif