#ifndef AUTOENCODER_H
#define AUTOENCODER_H

#include "mlp.h"

class Autoencoder:public MLP {
    private:
        unsigned char bottleneck_layer;
    public:
        double get_encoded(uint32_t index){return get_hidden(index,bottleneck_layer);}
        double get_decoded(uint32_t index){return get_output(index);}
        void set_encoded(uint32_t index,double value){layer[bottleneck_layer].neuron[index].h=value;}
        void decode(){feedforward(bottleneck_layer);}
        void encode(){feedforward(1,bottleneck_layer);}
        void sweep(){feedforward();autoencode();backpropagate();}
        // delete default constructor
        Autoencoder() = delete;
        // parametric constructor
        Autoencoder(uint32_t inputs, uint32_t bottleneck_neurons, unsigned char encoder_hidden_layers=1, unsigned char decoder_hidden_layers=1, ACT_FUNC act_func=F_SIGMOID, bool recurrent=true, double dropout=0){
            // add input layer
            add_layer(inputs,VANILLA,act_func);
            // add hidden layers for encoder section
            for (unsigned char n=0;n<encoder_hidden_layers;n++){
                uint32_t neurons=ceil(0.7*(layer[layers-1].neurons+bottleneck_neurons));
                add_layer(neurons,VANILLA,act_func);
            }
            // add bottleneck layer
            add_layer(bottleneck_neurons,VANILLA,act_func);
            bottleneck_layer=encoder_hidden_layers+1;
            // add hidden layers for decoder section
            for (unsigned char n=0;n<decoder_hidden_layers;n++){
                uint32_t neurons=ceil(0.7*(layer[layers-1].neurons+inputs));
                add_layer(neurons,VANILLA,act_func);
            }
            // add output layer
            add_layer(inputs,VANILLA,act_func);
            // set hyperparameters
            set_learning_rate_auto();
            set_recurrent(recurrent);
            set_dropout(dropout);
            set_training_mode(true);
        }
};

#endif