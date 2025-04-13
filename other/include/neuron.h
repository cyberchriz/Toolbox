// author: 'cyberchriz' (Christian Suer)
// this is a helper class that defines the properties of neurons as part of a layer (as part of a multilayer perceptron network)

// preprocessor directives
#ifndef NEURON_H
#define NEURON_H
#pragma once

#include <mlp_enums.h>
#include <vector>

// declaration of the 'Neuron' class
class Neuron{
    public:
        // core parameters
        bool dropout=false;             // dropout for overtraining mitigation
        double x=0;                     // receives sum of weighted inputs
        double h=0;                     // for result of activation function (=output of hidden neurons; also used for input value of initial layer)
        double scaled_label;            // used for output error calculation
        double output;                  // used for rescaled output (rescaling of h)
        int inputs;        
        OPTIMIZATION_METHOD opt_method;

        // recurrent inputs ("memory")    
        double m1=0;                    // stores h value from last iteration
        double m2=0;                    // stores rolling average with ratio 9:1 previous average versus new value for h
        double m3=0;                    // stores rolling average with ratio 99:1 previous average versus new value for h
        double m4=0;                    // stores rolling average with ratio 999:1 previous average versus new value for h

        // error calculations
        double gradient=0;              // used for output error (derivative of loss function) or hidden error, depending on the layer
        double  loss=0;                  // for MSE loss
        double loss_sum=0;              // used for average loss (after dividing by number of backprop iterations) 

        // values for input and label scaling
        double input_min=DBL_MAX;   // used for minmax scaling
        double input_max=-DBL_MAX;   // used for minmax scaling
        double input_maxabs=DBL_EPSILON;// used for input scaling
        double input_rolling_average = 0; // used for input scaling
        double input_mdev2 = 0;         // used for input scaling
        double input_variance = 1;      // used for input scaling, calculated as approximation from rolling mdev2
        double input_stddev=1;          // used for standardized scaling
        double label;                   // true/target values (before scaling)
        double label_min=DBL_MAX;   // used for label minmax scaling
        double label_max=-DBL_MAX;  // used for label minmax scaling
        double label_maxabs=DBL_EPSILON;// used for label scaling
        double label_rolling_average = 0; // used for label scaling
        double label_mdev2 = 0;         // used for label scaling
        double label_variance = 1;      // used for label scaling, calculated as approximation from rolling mdev2
        double label_stddev =1;         // used for standardized label scaling

        // weight matrix
        std::vector<double> input_weight;
        std::vector<double> input_weight_delta;
        std::vector<double> opt_v;                   // used for RMSprop, ADADELTA and ADAM
        std::vector<double> opt_w;                   // used for ADADELTA and ADAM
        
        // weights and weight-deltas for recurrent inputs
        double m1_weight;
        double m2_weight;
        double m3_weight;
        double m4_weight;
        double delta_m1=0;
        double delta_m2=0;
        double delta_m3=0;
        double delta_m4=0;    

        // bias weight
        double bias_weight;
        double delta_b=0;                 // calculated bias correction from backpropagation  

        // Constructor
        Neuron(int inputs)
        : inputs(inputs),
          input_weight(inputs, 0.0),
          input_weight_delta(inputs, 0.0),
          opt_v(inputs, 0.0),
          opt_w(inputs, 0.0),
          delta_b(0.0) {}
        
        // destructor
        ~Neuron(){};
};

#endif