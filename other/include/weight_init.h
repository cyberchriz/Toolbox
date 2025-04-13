#ifndef WEIGHT_INIT_H
#define WEIGHT_INIT_H

#include "distributions.h"
#include <cmath>

// weight initialization methods for neural networks
// author: 'cyberchriz' (Christian Suer)



// normal "Xavier" weight initialization (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_normal(int fan_in, int fan_out);

// uniform "Xavier" weight initializiation (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_uniform(int fan_in, int fan_out);

// uniform "Xavier" weight initialization for sigmoid activation
double f_Xavier_sigmoid(int fan_in, int fan_out);

// "Kaiming He" normal weight initialization, used for ReLU activation
double f_He_ReLU(int fan_in);

// modified "Kaiming He" nornal weight initialization, used for ELU activation
double f_He_ELU(int fan_in);






// DEFINITIONS:

// normal "Xavier" weight initialization (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_normal(int fan_in, int fan_out) {
    double result = Random<double>::gaussian(0.0, 1.0); // get a random number from a normal distribution with zero mean and variance one
    result *= sqrt(6 / sqrt(double(fan_in + fan_out)));
    return result;
}

// uniform "Xavier" weight initializiation (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_uniform(int fan_in, int fan_out) {
    double result = Random<double>::uniform(0.0, 1.0);
    result *= sqrt(2 / sqrt(double(fan_in + fan_out)));
    return result;
}

// uniform "Xavier" weight initialization for sigmoid activation
double f_Xavier_sigmoid(int fan_in, int fan_out) {
    double result = Random<double>::uniform(0.0, 1.0);
    result *= 4 * sqrt(6 / (double(fan_in + fan_out)));
    return result;
}

// "Kaiming He" normal weight initialization, used for ReLU activation
double f_He_ReLU(int fan_in) {
    double result = Random<double>::gaussian(0.0, 1.0); // get a random number from a normal distribution with zero mean and variance one
    result *= sqrt(2 / ((double)(fan_in)));
    return result;
}

// modified "Kaiming He" nornal weight initialization, used for ELU activation
double f_He_ELU(int fan_in) {
    double result = Random<double>::gaussian(0.0, 1.0);
    result *= sqrt(1.55 / (double(fan_in)));
    return result;
}

#endif