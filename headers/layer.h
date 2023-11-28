// author: 'cyberchriz' (Christian Suer)

#ifndef LAYER_H
#define LAYER_H

#include "activation_functions.h"
#include "mlp_enums.h"
#include "neuron.h"
#include <vector>

class Layer {
public:
    int neurons;
    OPTIMIZATION_METHOD opt_method;
    ACT_FUNC activation;
    int layer_dimensions;
    int input_dimensions;
    std::vector<Neuron> neuron;

    // Constructor
    Layer(int neurons, int inputs_per_neuron, OPTIMIZATION_METHOD opt_method = VANILLA, ACT_FUNC activation = F_TANH)
        : neurons(neurons),
        opt_method(opt_method),
        activation(activation),
        layer_dimensions(0),
        input_dimensions(0) {
        neuron.reserve(neurons);
        for (int i = 0; i < neurons; i++) {
            neuron.push_back(Neuron(inputs_per_neuron));
        }
    }

    // Copy constructor
    Layer(const Layer& other)
        : neurons(other.neurons),
          opt_method(other.opt_method),
          activation(other.activation),
          layer_dimensions(other.layer_dimensions),
          input_dimensions(other.input_dimensions),
          neuron(other.neuron) {
    }

    // Move constructor
    Layer(Layer&& other)
        : neurons(std::move(other.neurons)),
          opt_method(std::move(other.opt_method)),
          activation(std::move(other.activation)),
          layer_dimensions(std::move(other.layer_dimensions)),
          input_dimensions(std::move(other.input_dimensions)),
          neuron(std::move(other.neuron)) {
    }

    // Destructor
    ~Layer() {}
};


#endif