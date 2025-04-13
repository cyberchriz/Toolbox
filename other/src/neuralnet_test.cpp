#include "neuralnet.h"

int main(){
    Log::set_level(LOG_LEVEL_WARNING);

    // create Array of feature matrices and fill with random numbers
    int SAMPLES = 10000;
    int WIDTH = 5;
    Array<Array<double>> features;
    features = Array<Array<double>>({SAMPLES});
    for (int i=0; i<SAMPLES; i++){
        features.data[i] = Array<double>({WIDTH,WIDTH});
        features.data[i].fill_random_uniform();
    }

    // define model as autoencoder
    NeuralNet model;
    model.addlayer_input({WIDTH,WIDTH});
    model.addlayer_dense();
    model.addlayer_ReLU();
    model.addlayer_output({WIDTH,WIDTH},LossFunction::MSE);

    // set hyperparameters
    model.set_lr(0.001);
    model.set_scaling_method(ScalingMethod::MEAN_NORM);
    model.set_optimizer(OPTIMIZATION_METHOD::NESTEROV);
    model.set_gradient_clipping(false);

    model.log_summary();

    // train
    int BATCH_SIZE = 10;
    int EPOCHS = 10;    
    model.fit_set(features, features, BATCH_SIZE, EPOCHS);
    model.test_set(features, features);
}