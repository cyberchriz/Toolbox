[[return to main page]](../../../README.md)
# n-dimensional Arrays -<br>with many built-in math operations and sample statistics
usage: `#include <datastructures.h>` or include as part of `<datascience.h>`
___
# `class Array`
An instance of the class can be created by passing its dimensions as type `std::initializer_list<int>` or `std::vector<int>`to the constructor.

For example, for a 3x3x4 array of type `<double>` we can write:
```
Array<double> myArr{3,3,4};
```
___
Public Methods:

### constructors
```
Array(){};
Array(const std::initializer_list<int>& shape);
Array(const std::vector<int>& shape);
Array(const Array<int>& shape);
Array(const int elements);
Array(Array&& other) noexcept; // = move constructor
Array(const Array& other); // = copy constructor
```

### Getters & Setters
```
void set(const std::initializer_list<int>& index, const T value);
void set(const std::vector<int>& index, const T value);
void set(const Array<int>& index, const T value);
void set(const int row, const int col, const T value);
void set(const int index, const T value);
T get(const std::initializer_list<int>& index) const;
T get(const std::vector<int>& index) const;
T get(const Array<int>& index) const;
T get(const int row, const int col) const;
int get_dimensions() const;
int get_size(int dimension) const;
int get_size() const;
int get_elements() const;
std::vector<int> get_shape() const;
Array<int> get_convolution_shape(Array<int>& filter_shape, const bool padding=false);
std::vector<int> get_convolution_shape(std::vector<int>& filter_shape, const bool padding=false);
std::vector<int> get_stacked_shape();
std::string get_shapestring() const;
int get_subspace(int dimension) const;
std::vector<int> subspace();       
int get_capacity() const;    
int get_element(const std::initializer_list<int>& index) const;
int get_element(const std::vector<int>& index) const;
std::vector<int> get_index(int element) const;    
std::type_info const& get_type();
const char* get_typename();
```

### fill, initialize
```
void fill_values(const T value);
void fill_zeros();
void fill_identity();
void fill_random_gaussian(const T mu=0, const T sigma=1);
void fill_random_uniform(const T min=0, const T max=1.0);
void fill_random_binary(double ratio=0.5);
void fill_range(const T start=0, const T step=1);
void fill_dropout(double ratio=0.2);
void fill_Xavier_normal(int fan_in, int fan_out);
void fill_Xavier_uniform(int fan_in, int fan_out);
void fill_Xavier_sigmoid(int fan_in, int fan_out);
void fill_He_ReLU(int fan_in);
void fill_He_ELU(int fan_in);
```

### basic distribution properties
```
T min() const;
T max() const;
T mode() const;
double mean() const;
double median() const;
double variance() const;
double stddev() const;        
Array<double> nested_min() const;
Array<double> nested_max() const;
Array<double> nested_mean() const;
Array<double> nested_variance() const;                            
Array<double> nested_stddev() const;        
double skewness() const;
double kurtosis() const;
```

### addition
```
T sum() const;
Array<T> operator+(const T value) const;
Array<T> operator+(const Array<T>& other) const;
Array<T> operator++(int) const; //=postfix increment
Array<T>& operator++(); //=prefix increment
void operator+=(const T value);
void operator+=(const Array<T>& other);
```

### substraction
```
Array<T> operator-(const T value) const;
Array<T> operator-(const Array<T>& other) const;
Array<T> operator--(int) const; //=postfix decrement
Array<T>& operator--(); //=prefix decrement
void operator-=(const T value);
void operator-=(const Array<T>& other);
```

### multiplication
```
T product() const; // product reduction (=multiplying all elements with each other)
Array<T> operator*(const T factor) const; // elementwise multiplication with a scalar
void operator*=(const T factor); // alias for elementwise multiplication with a scalar
Array<T> tensordot(const Array<T>& other, const std::vector<int>& axes) const; //=tensor reduction
Array<T> tensordot(const Array<T>& other) const; //=tensor reduction
Array<T> operator*(const Array<T>& other) const; //=alias for tensordot matrix multiplication
void operator*=(const Array<T>& other) const; //=alias for tensordot matrix multiplication
T dotproduct(const Array<T>& other) const; //=scalar product
Array<T> Hadamard_product(const Array<T>& other) const;
```

### division
```
Array<T> operator/(const T quotient) const;
void operator/=(const T quotient);
Array<T> Hadamard_division(const Array<T>& other) const;
```

### modulo
```
void operator%=(const double num);
Array<double> operator%(const double num) const;
```

### exponentiation & logarithm
```
Array<T> pow(const T exponent);
Array<T> pow(const Array<T>& other);
Array<T> sqrt();
Array<T> log();
Array<T> log10();
```

### rounding (elementwise)
```
Array<T> round();
Array<T> floor();
Array<T> ceil();
Array<T> abs();
```

### min, max (elementwise comparison)
```
Array<T> min(const T value);
Array<T> max(const T value);
Array<T> min(const Array<T>& other);
Array<T> max(const Array<T>& other);
```

### trigonometric functions (elementwise)
```
Array<T> cos();
Array<T> sin();
Array<T> tan();
Array<T> acos();
Array<T> asin();
Array<T> atan();  
```

### hyperbolic functions (elementwise)
```
Array<T> cosh();
Array<T> sinh();
Array<T> tanh();
Array<T> acosh();
Array<T> asinh();
Array<T> atanh();                
```

### find, replace
```
Array<T> replace(const T old_value, const T new_value);
int find(const T value) const;
Array<char> sign();
```

### scale
```
Array<double> scale_minmax(T min=0,T max=1);
Array<double> scale_mean();
Array<double> scale_standardized();
Array<double> scale_unit_length();
```

### activation functions
```
Array<T> activation(ActFunc activation_function);
Array<T> derivative(ActFunc activation_function);
```
available activation functions:
 - RELU (rectified linear unit)
 - LRELU (leaky rectified linear unit)
 - ELU (exponential linar unit)
 - SIGMOID (=logistic)
 - TANH (hyperbolic tangent)
 - SOFTMAX (=normalized exponential)
 - IDENT (identity function)

### custom functions
```
Array<T> function(const T (*pointer_to_function)(T));
```

### outlier treatment
```
Array<T> outliers_truncate(double z_score=3);
Array<T> outliers_winsoring(double z_score=3);
Array<T> outliers_mean_imputation(double z_score=3);
Array<T> outliers_median_imputation(double z_score=3);
Array<T> outliers_value_imputation(T value=0, double z_score=3);        
```

### assignment
```
Array<T>& operator=(const Array<T>& other); // =copy assignment
Array<T>& operator=(Array<T>&& other) noexcept; // =move assignment
Array<T>& operator=(const T (&arr)[]); // =copy assignment
Array<T>& operator=(T (&&arr)[]) noexcept; // =move assignment
```

### elementwise comparison by single value
```
Array<bool> operator>(const T value) const;
Array<bool> operator>=(const T value) const;
Array<bool> operator==(const T value) const;
Array<bool> operator!=(const T value) const;
Array<bool> operator<(const T value) const;
Array<bool> operator<=(const T value) const;
```

### elementwise comparison with second array
```
Array<bool> operator>(const Array<T>& other) const;
Array<bool> operator>=(const Array<T>& other) const;
Array<bool> operator==(const Array<T>& other) const;
Array<bool> operator!=(const Array<T>& other) const;
Array<bool> operator<(const Array<T>& other) const;
Array<bool> operator<=(const Array<T>& other) const;
```

### elementwise logical operations
```
Array<bool> operator&&(const bool value) const;
Array<bool> operator||(const bool value) const;
Array<bool> operator!() const;
Array<bool> operator&&(const Array<T>& other) const;
Array<bool> operator||(const Array<T>& other) const;
```

### type casting
```
template<typename C> operator Array<C>();
```

### pointers
```
Array<T&> operator*(); // dereference operator
Array<T*> operator&(); // 'address-of' operator
```

### conversion
```
Array<T> flatten() const;
void reshape(std::vector<int> shape, const T init_value=0);
void reshape(std::initializer_list<int> shape, const T init_value=0);
void reshape(Array<int> shape, const T init_value=0);
Array<T> concatenate(const Array<T>& other, const int axis=0);
Array<T> add_dimension(int size, T init_value=0);
Array<T> padding(const int amount, const T value=0);
Array<T> padding_pre(const int amount, const T value=0);
Array<T> padding_post(const int amount, const T value=0);
Array<Array<T>> dissect(int axis);
Array<T> pool_max(const std::initializer_list<int> slider_shape, const std::initializer_list<int> stride_shape);     
Array<T> pool_avg(const std::initializer_list<int> slider_shape, const std::initializer_list<int> stride_shape); 
Array<T> convolution(const Array<T>& filter);    
Array<T> transpose() const;    
Array<T> reverse() const;
Array<T> stack();
Array<T> shuffle() const;
```

### 1d Array statistics
```
CorrelationResult<T> correlation(const Array<T>& other) const;
LinRegResult<T> regression_linear(const Array<T>& other) const;
PolyRegResult<T> regression_polynomial(const Array<T>& other, const int power) const;
HistogramResult<T> histogram(int bars) const;
```

public members of struc CorrelationResults (for Pearson, Spearman, ANOVA, covariance):
 - double x_mean, y_mean;  
 - double x_stddev, y_stddev;     
 - double y_intercept, slope;
 - double covariance;
 - double Pearson_R, Spearman_Rho;   
 - double r_squared;    
 - double RSS, SST, SSE, SSR, MSE, MSR; 
 - double ANOVA_F, ANOVA_p;
 - double z_score, t_score;          
 - Array<T> y_predict;
 - void print();

public members of struct LinRegResult:
 - double x_mean, y_mean=0;
 - double y_intercept, _slope;
 - double r_squared;
 - std::unique_ptr<double[]> _y_regression;
 - std::unique_ptr<double[]> _residuals;
 - double SST=0;
 - double SSR=0;
 - T predict(const T x);
 - bool is_good_fit(double threshold=0.95);
 - void print();

public members of struct PolyRegResult:
 - double SS_res, SS_tot;
 - double RSS, MSE, RSE;
 - double y_mean, x_mean;
 - double r_squared;
 - std::unique_ptr<double[]> coefficient;  
 - bool is_good_fit(double threshold=0.95){return r_squared>threshold;}
 - T predict(const T x);

public members of struct HistogramResult:
 - T min
 - T max
 - T bar_width
 - T _width
 - int bars
 - std::unique_ptr<Histogrambar[]> bar (with members: T lower_boundary, T upper_boundary, int abs_count, double rel_count)

### 1d Array dynamic handling
```
int push_back(T value);
T pop_last();
T pop_first();
T erase(const int index);
int grow(const int additional_elements);
int shrink(const int remove_amount);       
void resize(const int newsize);         
```

### 1d Array sample analysis
```
Array<int> ranking() const;
Array<T> exponential_smoothing(bool as_series=false) const;
double weighted_average(bool as_series=true) const;
double Dickey_Fuller(DIFFERENCING method=integer,double degree=1,double fract_exponent=2) const;
double Engle_Granger(const Array<T>& other) const;
Array<T> stationary(DIFFERENCING method=integer,double degree=1,double fract_exponent=2) const;
Array<T> sort(bool ascending=true) const;
double covariance(const Array<T>& other) const;
Array<T> binning(const int bins);                   
```
differencing methods for stationary transformation:
 - integer
 - logreturn
 - fractional
 - deltamean
 - original

### indexing
```
T& operator[](const int index) const;
T& operator[](const int index);
```

### output
```
void print(std::string comment="", std::string delimiter=", ", std::string line_break="\n", bool with_indices=false) const;
```

[[return to main page]](../../../README.md)