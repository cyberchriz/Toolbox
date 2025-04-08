#ifndef MLP_ENUMS_H
#define MLP_ENUMS_H

// available feature and label scaling methods
enum SCALING {
    none,              // no scaling
    standardized,      // standard deviation method (µ=0, sigma=1)
    normalized,        // minmax, range 0 to 1
    maxabs             // minmax, range -1 to +1
};

// available types of neural network layers
enum LAYER_TYPE {
    standard,
    recurrent,
    mod_recurrent,
    lstm,
    gru,
    convolutional,
    pooling
};

// neural network weight initialization methods
enum WEIGHT_INIT
{
    Xavier_normal = 1,     // Xavier Glorot normal
    Xavier_uniform = 2,    // Xavier Glorot uniform
    Sigmoid_uniform = 3,   // sigmoid uniform
    Kaiming_ReLU = 4,      // Kaiming He (ReLU)
    Kaiming_ELU = 5,       // Kaiming He (ELU)
    Kaiming_uniform = 6,   // Kaiming He uniform
    custom_uniform = 7     // custom uniform
};

// neural network loss functions
enum LOSS_FUNCTION
{
    MSE = 1,                                                         // mean squared error (MSE)
    CategoricalCrossEntropy = 2,                                     // categorical cross entropy
    BinaryCrossEntropy = 3,                                          // binary cross entropy
    MAE = 4                                                          // mean absolute error (MAE)
};

// available neural network optimization methods
enum OPTIMIZATION_METHOD {
    VANILLA,           // Vanilla Stochastic Gradient Descent
    MOMENTUM,          // Stochastic Gradient Descent with Momentum
    NESTEROV,          // Nesterov Accelerated Gradient (NAG)
    RMSPROP,           // RMSprop
    ADADELTA,          // ADADELTA
    ADAM,              // ADAM
    ADAGRAD            // AdaGrad
};

#endif