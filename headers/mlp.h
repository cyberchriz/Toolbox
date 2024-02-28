// author: 'cyberchriz' (Christian Suer)
// this is a neural network library for multilayer perceptrons (MLP) with flexible topologies


// preprocessor directives
#ifndef MLP_H
#define MLP_H
#include "activation_functions.h"
#include "layer.h"
#include "mlp_enums.h"
#include "neuron.h"
#include "rnd.h"
#include "weight_init.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

// MLP network class declaration
class MLP{
    public:
        int add_layer(int neurons, OPTIMIZATION_METHOD opt_method=ADADELTA, ACT_FUNC activation=F_LRELU);
        void reset_weights(uint32_t start_layer=1, uint32_t end_layer=UINT32_MAX, double factor=1.0);
        void feedforward(uint32_t start_layer=1, uint32_t end_layer=UINT32_MAX);
        void backpropagate();
        void set_input(uint32_t index, double value);
        double get_input(uint32_t index, uint32_t l=0){return layer[l].neuron[index].x;}
        double get_output(uint32_t index);
        double get_hidden(uint32_t index,uint32_t layer_index);
        void set_label(uint32_t index, double value);
        void autoencode();
        void save(std::string filename="network_backup.dat");
        bool network_exists() const { return layers != 0; }
        double get_loss_avg();
        void set_training_mode(bool active=true){training_mode=active;}
        void set_learning_rate(double value){base_lr=fmin(fmax(0,value),1.0);}
        void set_learning_rate_decay(uint32_t value){lr_decay=value;}
        void set_learning_momentum(double value){lr_momentum=fmin(fmax(0,value),1.0);}
        void set_learning_rate_auto(bool active=true){lr_auto=active;}
        void set_scaling_method(SCALING method=normalized){scaling_method=method;}
        void set_dropout(double value){dropout=fmax(0,fmin(value,1));}
        void set_recurrent(bool confirm){recurrent=confirm;}
        void set_gradient_clipping(bool active,double threshold=0.499){gradient_clipping=active;gradient_clipping_threshold=threshold;}
        double get_avg_h();
        double get_avg_output();     
        double get_lr(){return lr;}
        // constructor
        MLP(std::string filename="");
        // destructor
        ~MLP();    
    protected:
        int layers=0;
        std::vector<Layer> layer;
        void load(std::string filename);
        int backprop_iterations=0;
        SCALING scaling_method=none;
        double base_lr=0.005;
        double lr=base_lr;
        double lr_adjust_factor=1;
        const double lr_adjust_fraction=0.999;
        const double inv_fraction = 1/lr_adjust_fraction;
        double lr_momentum=0.0;
        double lr_decay=100000000;
        bool lr_auto=false;
        double opt_beta1=0.9;
        double opt_beta2=0.99;
        bool training_mode=true;
        bool gradient_clipping=false;
        double gradient_clipping_threshold=0.999;
        double dropout=0;
        bool recurrent=false;
        std::string filename;     
};

#endif