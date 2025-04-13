#ifndef ACTIVATION_FUNCTIONS_H
#define ACTIVATION_FUNCTIONS_H
#pragma once

#include <cmath>
#include <string>

// enumeration of available activation functions for neural networks
enum ACT_FUNC {
   F_IDENT,        // identity function
   F_SIGMOID,      // sigmoid (logistic)
   F_ELU,          // exponential linear unit (ELU)
   F_RELU,         // rectified linear unit (ReLU)
   F_LRELU,        // leaky ReLU
   F_TANH,         // hyperbolic tangent (tanh)
   F_OBLIQUE_TANH, // oblique tanh (custom)
   F_TANH_RECTIFIER,// tanh rectifier
   F_ARCTAN,       // arcus tangent (arctan)
   F_ARSINH,       // area sin. hyperbolicus (inv. hyperbol. sine)
   F_SOFTSIGN,     // softsign (Elliot)
   F_ISRU,         // inverse square root unit (ISRU)
   F_ISRLU,        // inv.squ.root linear unit (ISRLU)
   F_SOFTPLUS,     // softplus
   F_BENTIDENT,    // bent identity
   F_SINUSOID,     // sinusoid
   F_SINC,         // cardinal sine (sinc)
   F_GAUSSIAN,     // gaussian normal
   F_DIFFERENTIABLE_HARDSTEP, // differentiable hardstep
   F_LEAKY_DIFF_HARDSTEP, // leaky differentiable hardstep
   F_SOFTMAX,      // normalized exponential (softmax)
   F_OBLIQUE_SIGMOID, // oblique sigmoid
   F_LOG_RECTIFIER, // log rectifier
   F_LEAKY_LOG_RECTIFIER, // leaky log rectifier
   F_RAMP          // ramp
                   // note: the softmax function can't be part of this library because it has no single input but needs the other neurons of the layer, too, so it
  };               //       needs to be defined from within the neural network code

class Activation {
public:
    // public methods
    static double function(double z, ACT_FUNC f);
    static double derivative(double z, ACT_FUNC f);
    static std::string to_string(ACT_FUNC f);
private:
    Activation() {};
    ~Activation() {};
    // functions
    static double sigmoid(double z);
    static double ELU(double z);
    static double ReLU(double z);
    static double LReLU(double z);
    static double modtanh(double z);
    static double oblique_tanh(double z);
    static double tanh_rectifier(double z);
    static double arctan(double z);
    static double arsinh(double z);
    static double softsign(double z);
    static double ISRU(double z);
    static double ISRLU(double z);
    static double softplus(double z);
    static double bentident(double z);
    static double sinusoid(double z);
    static double sinc(double z);
    static double gaussian(double z);
    static double diff_hardstep(double z);
    static double leaky_diff_hardstep(double z);
    static double oblique_sigmoid(double z);
    static double log_rectifier(double z);
    static double leaky_log_rectifier(double z);
    static double ramp(double z);
    // derivatives
    static double sigmoid_drv(double z);
    static double ELU_drv(double z);
    static double ReLU_drv(double z);
    static double LReLU_drv(double z);
    static double modtanh_drv(double z);
    static double oblique_tanh_drv(double z);
    static double tanh_rectifier_drv(double z);
    static double arctan_drv(double z);
    static double arsinh_drv(double z);
    static double softsign_drv(double z);
    static double ISRU_drv(double z);
    static double ISRLU_drv(double z);
    static double softplus_drv(double z);
    static double bentident_drv(double z);
    static double sinusoid_drv(double z);
    static double sinc_drv(double z);
    static double gaussian_drv(double z);
    static double diff_hardstep_drv(double z);
    static double leaky_diff_hardstep_drv(double z);
    static double oblique_sigmoid_drv(double z);
    static double log_rectifier_drv(double z);
    static double leaky_log_rectifier_drv(double z);
    static double ramp_drv(double z);
};

#endif