// author: 'cyberchriz' (Christian Suer)
// license: 'github.com/cyberchriz/git/LICENCE.md'

// objective of this file: flexible implementation of sequential modular neural networks
// with a variety of different layer types, making use of matrix operations

#ifndef NEURALNET_H
#define NEURALNET_H
#pragma once

#include <datastructures.h>
#include <distributions.h>
#include <log.h>
#include <initializer_list>
#include <vector>

// forward declarations
class NeuralNet;

enum LossFunction{
    MSE,                // Mean Squared Error
    MAE,                // Mean Absolute Error
    MAPE,               // Mean Absolute Percentage Error
    MSLE,               // Mean Squared Logarithmic Error
    COS_PROX,            // Cosine Proximity
    CAT_CROSS_ENTR,       // Categorical Crossentropy
    SPARSE_CAT_CROSS_ENTR, // Sparse Categorical Crossentropy
    BIN_CROSS_ENTR,       // Binary Crossentropy
    KLD,                // Kullback-Leibler Divergence
    POISSON,            // Poisson
    HINGE,              // Hinge
    SQUARED_HINGE,       // Squared Hinge
    LOG_COSH,            // LogCosh
    LOSS_FUNCTION_COUNT
};

enum LayerType{
    POOL_LAYER,
    INPUT_LAYER,
    OUTPUT_LAYER,
    LSTM_LAYER,
    RECURRENT_LAYER,
    DENSE_LAYER,
    CONVOLUTIONAL_LAYER,
    GRU_LAYER,
    DROPOUT_LAYER,
    RELU_LAYER,
    LRELU_LAYER,
    ELU_LAYER,
    SIGMOID_LAYER,
    TANH_LAYER,
    FLATTEN_LAYER,
    LAYER_TYPE_COUNT, // used to verify valid enum argument
    LAYER_TYPE_NONE
};

enum ScalingMethod{
    STANDARDIZED,
    MIN_MAX_NORM,
    MEAN_NORM,
    NO_SCALING
};

enum OPTIMIZATION_METHOD {
   VANILLA,           // Vanilla Stochastic Gradient Descent
   MOMENTUM,          // Stochastic Gradient Descent with Momentum
   NESTEROV,          // Nesterov Accelerated Gradient (NAG)
   RMSPROP,           // RMSprop
   ADADELTA,          // ADADELTA
   ADAM,              // ADAM
   ADAGRAD            // AdaGrad
};

struct Layer{
    public:
        // public member objects
        LayerType type;
        int neurons=0;
        int dimensions;
        int timesteps;
        bool is_stacked=false;
        bool padding=false;
        Array<Array<double>> feature_stack_h;
        Array<Array<double>> gradient_stack;
        Array<Array<double>> filter_stack;
        std::vector<int> filter_shape;
        std::vector<int> shape;
        Array<double> h; // hidden states
        Array<double> label; // labels for output layer
        Array<double> gradient;
        Array<double> loss;
        Array<double> loss_sum;
        Array<Array<double>> gradient_t; // vector of timesteps holding the gradients for h_t
        Array<Array<double>> x_t; // vector of timesteps holding the input states for RNN, LSTM
        Array<Array<double>> h_t; // vector of timesteps holding the hidden states for LSTMs
        Array<Array<double>> c_t; // vector of timesteps holding the cell states for LSTMs
        Array<Array<double>> U;   // stores the weights for h(t-1) for RNN
        Array<Array<double>> U_f; // stores the weights for h(t-1) to calculate the forget gate: f(t) = σ(W_f * x(t) + U_f * h(t-1) + b_f)
        Array<Array<double>> U_i; // stores the weights for h(t-1) to calculate the input gate: i(t) = σ(W_i * x(t) + U_i * h(t-1) + b_i)
        Array<Array<double>> U_o; // stores the weights for h(t-1) to calculate the output gate: o(t) = σ(W_o * x(t) + U_o * h(t-1) + b_o)
        Array<Array<double>> U_c; // stores the weights for h(t-1) to calculate the candidate gate: c(t) = tanh(W_c * x(t) + U_c * h(t-1) + b_c)
        Array<Array<double>> U_z; // stores the weights for h(t-1) to calculate the update gate: z(t) = σ(W_z * x(t) + U_z + h(t-1) + b_z)
        Array<Array<double>> U_r; // stores the weights for h(t-1) to calculate the reset gate: r(t) = σ(W_r * x(t) + U_r + h(t-1) + b_r)
        Array<Array<double>> W_f; // forget gate weights for x(t)
        Array<Array<double>> W_i; // input gate weights for x(t)
        Array<Array<double>> W_o; // output gate weights for x(t)
        Array<Array<double>> W_c; // candidate gate weights for x(t)
        Array<Array<double>> W_z; // update gate weights for x(t)
        Array<Array<double>> W_r; // reset gate weights for x(t)
        Array<Array<double>> W_x; // stores the weights for dense connections (dense layers, output layers)
        Array<Array<double>> delta_W_x; // stores the weight deltas for dense connections in order to support momentum optimization
        Array<Array<double>> opt_v;
        Array<Array<double>> opt_w;
        Array<Array<double>> f_gate_t;
        Array<Array<double>> i_gate_t;
        Array<Array<double>> o_gate_t;
        Array<Array<double>> c_gate_t;
        Array<Array<double>> z_gate_t; // update gate for GRU
        Array<Array<double>> r_gate_t; // reset gate for GRU
        Array<double> b; // stores the bias matrix for the given layer
        Array<double> b_f; // forget gate bias weights;
        Array<double> b_i; // input gate bias weights;
        Array<double> b_o; // output gate bias weights;
        Array<double> b_c; // candidate gate bias weights;
        Array<double> b_z; // update gate bias weights;
        Array<double> b_r; // reset gate bias weights;
        // weights for dense connections
        Array<Array<double *>> W_out; // pointers to W_i weights of next layer
        double dropout_ratio=0;
        std::vector<int> pooling_slider_shape; // vector of pointers to pooling slider shapes
        std::vector<int> pooling_stride_shape; // vector of pointers to pooling stride shapes
        PoolMethod pooling_method;

        // default constructor
        Layer(){};

        // Copy constructor
        Layer(const Layer& other) {
            this->type = other.type;
            this->neurons = other.neurons;
            this->dimensions = other.dimensions;
            this->timesteps = other.timesteps;
            this->is_stacked = other.is_stacked;
            this->feature_stack_h = other.feature_stack_h;
            this->gradient_stack = other.gradient_stack;
            this->filter_stack = other.filter_stack;
            this->filter_shape = other.filter_shape;
            this->shape = other.shape;
            this->h = other.h;
            this->label = other.label;
            this->gradient = other.gradient;
            this->loss = other.loss;
            this->loss_sum = other.loss_sum;
            this->gradient_t = other.gradient_t;
            this->x_t = other.x_t;
            this->h_t = other.h_t;
            this->c_t = other.c_t;
            this->U = other.U;
            this->U_f = other.U_f;
            this->U_i = other.U_i;
            this->U_o = other.U_o;
            this->U_c = other.U_c;
            this->U_z = other.U_z;
            this->U_r = other.U_r;
            this->W_f = other.W_f;
            this->W_i = other.W_i;
            this->W_o = other.W_o;
            this->W_c = other.W_c;
            this->W_z = other.W_z;
            this->W_r = other.W_r;
            this->W_x = other.W_x;
            this->delta_W_x = other.delta_W_x;
            this->opt_v = other.opt_v;
            this->opt_w = other.opt_w;
            this->f_gate_t = other.f_gate_t;
            this->i_gate_t = other.i_gate_t;
            this->o_gate_t = other.o_gate_t;
            this->c_gate_t = other.c_gate_t;
            this->z_gate_t = other.z_gate_t;
            this->r_gate_t = other.r_gate_t;
            this->b = other.b;
            this->b_f = other.b_f;
            this->b_i = other.b_i;
            this->b_o = other.b_o;
            this->b_c = other.b_c;
            this->b_z = other.b_z;
            this->b_r = other.b_r;
            this->W_out = other.W_out;
            this->dropout_ratio = other.dropout_ratio;
            this->pooling_slider_shape = other.pooling_slider_shape;
            this->pooling_stride_shape = other.pooling_stride_shape;
            this->pooling_method = other.pooling_method;
        }

        // Move constructor
        Layer(Layer&& other) noexcept {
            this->type = std::move(other.type);
            this->neurons = std::move(other.neurons);
            this->dimensions = std::move(other.dimensions);
            this->timesteps = std::move(other.timesteps);
            this->is_stacked = std::move(other.is_stacked);
            this->feature_stack_h = std::move(other.feature_stack_h);
            this->gradient_stack = std::move(other.gradient_stack);
            this->filter_stack = std::move(other.filter_stack);
            this->filter_shape = std::move(other.filter_shape);
            this->shape = std::move(other.shape);
            this->h = std::move(other.h);
            this->label = std::move(other.label);
            this->gradient = std::move(other.gradient);
            this->loss = std::move(other.loss);
            this->loss_sum = std::move(other.loss_sum);
            this->gradient_t = std::move(other.gradient_t);
            this->x_t = std::move(other.x_t);
            this->h_t = std::move(other.h_t);
            this->c_t = std::move(other.c_t);
            this->U = std::move(other.U);
            this->U_f = std::move(other.U_f);
            this->U_i = std::move(other.U_i);
            this->U_o = std::move(other.U_o);
            this->U_c = std::move(other.U_c);
            this->U_z = std::move(other.U_z);
            this->U_r = std::move(other.U_r);
            this->W_f = std::move(other.W_f);
            this->W_i = std::move(other.W_i);
            this->W_o = std::move(other.W_o);
            this->W_c = std::move(other.W_c);
            this->W_z = std::move(other.W_z);
            this->W_r = std::move(other.W_r);
            this->W_x = std::move(other.W_x);
            this->delta_W_x = std::move(other.delta_W_x);
            this->opt_v = std::move(other.opt_v);
            this->opt_w = std::move(other.opt_w);
            this->f_gate_t = std::move(other.f_gate_t);
            this->i_gate_t = std::move(other.i_gate_t);
            this->o_gate_t = std::move(other.o_gate_t);
            this->c_gate_t = std::move(other.c_gate_t);
            this->z_gate_t = std::move(other.z_gate_t);
            this->r_gate_t = std::move(other.r_gate_t);
            this->b = std::move(other.b);
            this->b_f = std::move(other.b_f);
            this->b_i = std::move(other.b_i);
            this->b_o = std::move(other.b_o);
            this->b_c = std::move(other.b_c);
            this->b_z = std::move(other.b_z);
            this->b_r = std::move(other.b_r);
            this->W_out = std::move(other.W_out);
            this->dropout_ratio = std::move(other.dropout_ratio);
            this->pooling_slider_shape = std::move(other.pooling_slider_shape);
            this->pooling_stride_shape = std::move(other.pooling_stride_shape);
            this->pooling_method = std::move(other.pooling_method);
        }


        // destructor
        ~Layer(){};
};

class NeuralNet{
    public:
        // public methods
        void fit_set(const Array<Array<double>>& features, const Array<Array<double>>& labels, const int batch_size, const int epochs, bool batch_gradient_averaging=true); // for batch training
        void fit_single(const Array<double>& features, const Array<double>& labels); // for online training
        double test_set(const Array<Array<double>>& features, const Array<Array<double>>& labels); // returns the accuracy of a test or validation set
        void autoencode(const Array<Array<double>>& features, const int batch_size, const int epochs); // for autoencoder batch training
        void autoencode(const Array<double>& features); // for online autoencoder training
        Array<double> predict(const Array<double>& features, bool rescale=true); // predict output from new feature input
        void save(); // save model to file
        void load(); // load model from file
        void log_summary(LogLevel level=LOG_LEVEL_INFO); // logs a summary of the model architecture     
        void set_scaling_method(ScalingMethod method){scaling_method = method;}
        void set_optimizer(OPTIMIZATION_METHOD method){opt_method = method;}
        void set_lr(const double value){lr = std::fmin(1,std::fmax(0,value));}
        void set_momentum(const double value){momentum = std::fmin(1,std::fmax(0,value));}
        void set_loss_function(const LossFunction func){loss_function = func;}
        void set_gradient_clipping(const bool active=true, const double limit=0.49999){gradient_clipping=active; max_gradient = std::fmax(0,limit);}
        void set_feature_maps(const int n){if (layers==0 || (layers>0 && layer[layers-1].type!=OUTPUT_LAYER)) {feature_maps = n;}}

        void addlayer_input(std::vector<int> shape);
        void addlayer_input(const int neurons){std::vector<int> shape = {neurons}; addlayer_input(shape);}
        void addlayer_output(std::vector<int> shape, LossFunction loss_function=MSE);
        void addlayer_output(const int neurons, LossFunction loss_function=MSE){std::vector<int> shape = {neurons};addlayer_output(shape, loss_function);}
        void addlayer_sigmoid();
        void addlayer_ReLU();
        void addlayer_lReLU();
        void addlayer_ELU();
        void addlayer_tanh();         
        void addlayer_pool(const PoolMethod method, const std::vector<int> slider_shape={2,2}, const std::vector<int> stride_shape={2,2});               
        void addlayer_lstm(std::vector<int> shape, const int timesteps);
        void addlayer_lstm(const int neurons, const int timesteps=10){addlayer_lstm({neurons},timesteps);}
        void addlayer_lstm(const int timesteps=10){addlayer_lstm(layer[layers-1].shape,timesteps);}
        void addlayer_recurrent(std::vector<int> shape, const int timesteps=10);
        void addlayer_recurrent(const int neurons, const int timesteps=10){addlayer_recurrent({neurons}, timesteps);}
        void addlayer_recurrent(const int timesteps=10){addlayer_recurrent(layer[layers-1].shape,timesteps);}
        void addlayer_dense(std::vector<int> shape);
        void addlayer_dense(const int neurons){std::vector<int> layer_shape = {neurons}; addlayer_dense({layer_shape});}
        void addlayer_dense(){addlayer_dense(layer[layers-1].shape);}
        void addlayer_convolutional(const int filter_radius=1, bool padding=false);
        void addlayer_GRU(std::vector<int> shape, const int timesteps=10);
        void addlayer_GRU(const int neurons, const int timesteps=10){addlayer_GRU({neurons}, timesteps);}
        void addlayer_GRU(const int timesteps=10){addlayer_GRU(layer[layers-1].shape, timesteps);}
        void addlayer_dropout(const double ratio=0.2);
        void addlayer_flatten();

        // constructor(s)
        NeuralNet(){}
        // destructor
        ~NeuralNet(){};
    private:
        // private methods
        void backpropagate(bool batch_gradient_averaging=true);
        void scale_labels(const Array<double>& raw_labels);
        void calculate_loss();
        void layer_init(LayerType type, std::vector<int> shape);
        void layer_make_dense_connections();       
            
        // private members
        std::vector<Layer> layer;
        int layers=0;
        double loss_avg=0;
        LossFunction loss_function=LossFunction::MSE;
        int backprop_iterations=0;
        int forward_iterations=0;
        int sample_counter=0;
        int loss_counter=0;    
        Array<double> features_mean;
        Array<double> features_stddev;
        Array<double> features_min;
        Array<double> features_max;
        Array<double> labels_mean;
        Array<double> labels_stddev;
        Array<double> labels_min;
        Array<double> labels_max;        
        ScalingMethod scaling_method = STANDARDIZED;
        double lr = 0.001;
        double momentum = 0.9;
        OPTIMIZATION_METHOD opt_method = VANILLA;
        bool gradient_clipping = false;
        double max_gradient;
        int feature_maps=10;
};

#endif