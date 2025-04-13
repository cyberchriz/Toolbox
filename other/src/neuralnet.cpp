#include "neuralnet.h"

// batch training
void NeuralNet::fit_set(const Array<Array<double>>& features, const Array<Array<double>>& labels, const int batch_size, const int epochs, bool batch_gradient_averaging){
    int total_samples = features.get_elements();
    int batches = std::ceil(total_samples/batch_size);
    // get scaling parameters from entire dataset
    features_min = features.nested_min();
    features_max = features.nested_max();
    labels_min = labels.nested_min();
    labels_max = labels.nested_max();
    features_mean = features.nested_mean();
    labels_mean = labels.nested_mean();
    features_stddev = features.nested_stddev();
    labels_stddev = labels.nested_stddev();

    // iterate over epochs
    for (int epoch = 1; epoch<=epochs; epoch++){
        int batch = 1;
        // iterate over samples
        for (int sample = 0; sample < total_samples; sample++){
            // process samples for current batch batch
            if (sample_counter < batch_size){
                // run forward and backward pass
                static Array<double> rescaled_results = Array<double>({labels.data[0].get_elements()});
                rescaled_results = predict(features[sample],true);
                scale_labels(labels[sample]);
                calculate_loss();
            }
            // finalize batch
            else {
                Log::log(LOG_LEVEL_INFO, "epoch ", epoch, "/", epochs, ", batch ", batch, "/", batches, " FINISHED, average loss (across all outputs) ", loss_avg);
                backpropagate(batch_gradient_averaging);
                // reset loss_counter and loss_sum at end of mini-batch
                loss_counter = 0;
                layer[layers-1].loss_sum.fill_zeros();
                batch++;
            }            
        }
        Log::log(LOG_LEVEL_NONE, "epoch ", epoch, "/", epochs, " FINISHED; total number of completed backprop iterations: ", backprop_iterations);        
    }
}

// single iteration online training;
// note: always perform batch training first,
// otherwise proper scaling isn't possible
void NeuralNet::fit_single(const Array<double>& features, const Array<double>& labels){
    if (backprop_iterations==0){return;}

    // update scaling parameters
    static double update_quota = 0.005;
    features_min = features.min(features_min);
    features_max = features.max(features_max);
    labels_min = labels.min(labels_min);
    labels_max = labels.max(labels_max);
    features_mean = (features_mean * (1-update_quota)) + (features * update_quota);
    labels_mean = (labels_mean * (1-update_quota)) + (labels * update_quota);
    static Array<double> features_variance = Array<double>(features.get_shape());
    static Array<double> labels_variance = Array<double>(labels.get_shape());
    features_variance = features_stddev.pow(2.0f);
    labels_variance = labels_stddev.pow(2.0f);
    features_variance = (features_variance * (1-update_quota)) + ((features-features_mean).pow(2.0f) * update_quota);
    labels_variance = (labels_variance * (1-update_quota)) + ((labels-labels_mean).pow(2.0f) * update_quota);
    features_stddev = features_variance.sqrt();
    labels_stddev = labels_variance.sqrt();

    // execute forward and backward pass
    static Array<double> rescaled_results = Array<double>({labels.get_elements()});
    rescaled_results = predict(features,false);
    scale_labels(labels);
    calculate_loss();
    backpropagate();
}

void NeuralNet::autoencode(const Array<Array<double>>& features, const int batch_size, const int epochs){
    fit_set(features,features,batch_size,epochs);
}

void NeuralNet::autoencode(const Array<double>& features){
    fit_single(features,features);
}

double NeuralNet::test_set(const Array<Array<double>>& features, const Array<Array<double>>& labels){
    int total_samples = features.get_elements();
    Array<double> bias = Array<double>(labels.data[0].get_shape());
    Array<double> correct_predictions = Array<double>(labels.data[0].get_shape());
    bias.fill_zeros();
    correct_predictions.fill_zeros();
    sample_counter=0;
    layer[layers-1].gradient.fill_zeros();
    // iterate over test samples
    for (int sample = 0; sample < total_samples; sample++){
        static Array<double> rescaled_results = Array<double>({labels.data[0].get_elements()});
        rescaled_results = predict(features[sample],true);
        scale_labels(labels[sample]);                               
        calculate_loss();
        bias += rescaled_results.Hadamard_division(labels[sample]);
        correct_predictions += (rescaled_results > 0.5) && (labels[sample] >0.5);
    }
    // output results
    double regression_accuracy = bias.mean() / layer[layers-1].neurons - 1.0;
    double classification_accuracy = correct_predictions.sum() / (total_samples * layer[layers-1].neurons);
    Log::log(LOG_LEVEL_NONE, "TEST RESULTS:\n",
                             " - regression accuracy (relative bias): ", regression_accuracy*100, "%\n",
                             " - classification accuracy (correct predictions over number of samples, with >0.5 as threshold for 'true'): ", classification_accuracy*100, "%\n",
                             " - average loss (mean across all output neurons): ", loss_avg);
    // reset loss_counter and loss_sum
    loss_counter = 0;
    layer[layers-1].loss_sum.fill_zeros();
       
    return std::max(regression_accuracy, classification_accuracy);
}


// feedforward, i.e. predict output from new feature input
Array<double> NeuralNet::predict(const Array<double>& features, bool rescale){
    forward_iterations++;
    // iterate over layers
    for (int l=0;l<layers;l++){
        switch (layer[l].type){

            case INPUT_LAYER:  {
                // scale and assign features
                if (scaling_method==ScalingMethod::MIN_MAX_NORM){
                    layer[l].h = (features-features_min).Hadamard_division(features_max-features_min);
                }
                if (scaling_method==ScalingMethod::MEAN_NORM){
                    layer[l].h = (features-features_mean).Hadamard_division(features_max-features_min);
                }
                if (scaling_method==ScalingMethod::STANDARDIZED){
                    layer[l].h = (features-features_mean).Hadamard_division(features_stddev);
                }                
            } break;

            case POOL_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] =
                            layer[l-1].feature_stack_h[n].pool(layer[l].pooling_method, layer[l].pooling_slider_shape, layer[l].pooling_stride_shape);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.pool(layer[l].pooling_method, layer[l].pooling_slider_shape, layer[l].pooling_stride_shape);
                }
            } break;

            case DENSE_LAYER:
            case OUTPUT_LAYER: {
                // iterate over elements
                for (int j=0;j<layer[l].neurons;j++){
                    // assign dotproduct of weight matrix * inputs, plus bias
                    std::vector<int> index = layer[l].h.get_index(j);
                    layer[l].h.set(j,layer[l-1].h.dotproduct(layer[l].W_x.get(index)) + layer[l].b[j]);
                }                
            } break;

            case LSTM_LAYER: {
                int t = layer[l].timesteps-1;
                // get current inputs x(t)
                layer[l].x_t.pop_first();
                layer[l].x_t.push_back(layer[l-1].h);            
                // shift c_t, h_t and gates by 1 timestep
                layer[l].c_t.pop_first();
                layer[l].h_t.pop_first();
                // roll timesteps for gates
                layer[l].f_gate_t.push_back(layer[l].f_gate_t.pop_first());
                layer[l].i_gate_t.push_back(layer[l].i_gate_t.pop_first());
                layer[l].o_gate_t.push_back(layer[l].o_gate_t.pop_first());
                layer[l].c_gate_t.push_back(layer[l].c_gate_t.pop_first());
                // calculate gate inputs
                for (int j=0;j<layer[l].neurons;j++){
                    layer[l].f_gate_t[t].set(j,layer[l].W_f[j].dotproduct(layer[l].x_t[t]) + layer[l].U_f[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_f[j]);
                    layer[l].i_gate_t[t].set(j,layer[l].W_i[j].dotproduct(layer[l].x_t[t]) + layer[l].U_i[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_i[j]);
                    layer[l].o_gate_t[t].set(j,layer[l].W_o[j].dotproduct(layer[l].x_t[t]) + layer[l].U_o[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_o[j]);
                    layer[l].c_gate_t[t].set(j,layer[l].W_c[j].dotproduct(layer[l].x_t[t]) + layer[l].U_c[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_c[j]);
                }
                // activate gates
                layer[l].f_gate_t[t] = layer[l].f_gate_t[t].activation(ActFunc::SIGMOID);
                layer[l].i_gate_t[t] = layer[l].i_gate_t[t].activation(ActFunc::SIGMOID);
                layer[l].o_gate_t[t] = layer[l].o_gate_t[t].activation(ActFunc::SIGMOID);
                layer[l].c_gate_t[t] = layer[l].c_gate_t[t].activation(ActFunc::TANH);
                // add c_t and h_t results for current timestep
                layer[l].c_t.push_back(layer[l].f_gate_t[t].Hadamard_product(layer[l].c_t[t-1]) + layer[l].i_gate_t[t].Hadamard_product(layer[l].c_gate_t[t]));
                layer[l].h_t.push_back(layer[l].o_gate_t[t].Hadamard_product(layer[l].c_t[t].activation(ActFunc::TANH)));
                layer[l].h = layer[l].h_t[t];
                } break;

            case RECURRENT_LAYER: {
                // define 't' as current timestep
                int t = layer[l].timesteps-1;
                
                // roll input vector x_t by 1 timestep and update the current input
                layer[l].x_t.pop_first();
                layer[l].x_t.push_back(layer[l-1].h);
                
                // roll output h_t by 1 timestep
                layer[l].h_t.push_back(layer[l].h_t.pop_first());
                
                // iterate over all neurons of the recurrent layer and compute the current outputs h_t
                for (int j=0;j<layer[l].neurons;j++){
                    // set h(t) = x(t)*W_x + h(t-1)*U + bias
                    layer[l].h_t[t][j] = layer[l].x_t[t].dotproduct(layer[l].W_x[j]) + layer[l].h_t[t-1].dotproduct(layer[l].U[j]) + layer[l].b[j];
                }
                layer[l].h = layer[l].h_t[t];
            } break;
                
            case CONVOLUTIONAL_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].convolution(layer[l].filter_stack[n], layer[l].padding);
                    }
                }
                else {
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].h.convolution(layer[l].filter_stack[n], layer[l].padding);
                    }
                }
                if (!layer[l+1].is_stacked){
                    layer[l].h = layer[l].feature_stack_h.stack();
                }                 
            } break;
                
            case GRU_LAYER: {
                int t = layer[l].timesteps-1;
                // get current inputs x(t)
                layer[l].x_t.pop_first();
                layer[l].x_t.push_back(layer[l-1].h);            
                // shift c_t and h_t by 1 timestep
                layer[l].h_t.pop_first();
                // roll gates
                layer[l].z_gate_t.push_back(layer[l].z_gate_t.pop_first());
                layer[l].r_gate_t.push_back(layer[l].r_gate_t.pop_first());
                layer[l].c_gate_t.push_back(layer[l].c_gate_t.pop_first());
                // calculate gate inputs
                for (int j=0; j<layer[l].neurons; j++){
                    layer[l].z_gate_t[t].set(j,layer[l].W_z[j].dotproduct(layer[l].x_t[t]) + layer[l].U_z[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_z[j]);
                    layer[l].r_gate_t[t].set(j,layer[l].W_r[j].dotproduct(layer[l].x_t[t]) + layer[l].U_r[j].dotproduct(layer[l].h_t[t-1]) + layer[l].b_r[j]);
                    layer[l].c_gate_t[t].set(j,layer[l].W_c[j].dotproduct(layer[l].x_t[t]) + layer[l].U_c[j].dotproduct(layer[l].r_gate_t[t].Hadamard_product(layer[l].h_t[t-1])) + layer[l].b_c[j]);                    
                }
                // activate gates
                layer[l].z_gate_t[t] = layer[l].z_gate_t[t].activation(ActFunc::SIGMOID);
                layer[l].r_gate_t[t] = layer[l].r_gate_t[t].activation(ActFunc::SIGMOID);
                layer[l].c_gate_t[t] = layer[l].c_gate_t[t].activation(ActFunc::TANH);
                layer[l].h_t[t] = ((layer[l].z_gate_t[t]-1)*-1).Hadamard_product(layer[l].h_t[t-1]) + layer[l].z_gate_t[t].Hadamard_product(layer[l].c_gate_t[t]);
                layer[l].h = layer[l].h_t[t];
            } break;
                
            case DROPOUT_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n];
                        layer[l].feature_stack_h[n].fill_dropout(layer[l].dropout_ratio);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h;
                    layer[l].h.fill_dropout(layer[l].dropout_ratio);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case RELU_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].activation(ActFunc::RELU);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.activation(ActFunc::RELU);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case LRELU_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].activation(ActFunc::LRELU);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.activation(ActFunc::LRELU);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case ELU_LAYER: {
                                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].activation(ActFunc::ELU);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.activation(ActFunc::ELU);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case SIGMOID_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].activation(ActFunc::SIGMOID);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.activation(ActFunc::SIGMOID);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case TANH_LAYER: {
                if (layer[l-1].is_stacked){
                    for (int n=0; n<feature_maps; n++){
                        layer[l].feature_stack_h[n] = layer[l-1].feature_stack_h[n].activation(ActFunc::TANH);
                    }
                    if (!layer[l+1].is_stacked){
                        layer[l].h = layer[l].feature_stack_h.stack();
                    }
                }
                else {
                    layer[l].h = layer[l-1].h.activation(ActFunc::TANH);
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            case FLATTEN_LAYER: {
                if (layer[l-1].is_stacked){
                    layer[l].h = layer[l-1].feature_stack_h.stack().flatten();
                }
                else {
                    layer[l].h = layer[l-1].h.flatten();
                    if (Log::at_least(LOG_LEVEL_DEBUG)){
                        layer[l].h.print("layer["+std::to_string(l)+"].h: "," ");
                    }                    
                }
            } break;
                
            default: {
                layer[l].h = layer[l-1].h;
            } break;
        }        
    }
    // return output
    if (rescale && scaling_method!=ScalingMethod::NO_SCALING){
        if (scaling_method == ScalingMethod::MIN_MAX_NORM){
            return layer[layers-1].h.Hadamard_product(labels_max-labels_min)+labels_min;
        }
        if (scaling_method == ScalingMethod::MEAN_NORM){
            return layer[layers-1].h.Hadamard_product(labels_max-labels_min)+labels_mean;
        }
        if (scaling_method == ScalingMethod::STANDARDIZED){
            return layer[layers-1].h.Hadamard_product(labels_stddev)+labels_mean;
        }
    }
    return layer[layers-1].h;
}

// save the model to a file
void NeuralNet::save(){
    // TODO
}

// load the model from a file
void NeuralNet::load(){
    // TODO
}

// prints a summary of the model architecture
void NeuralNet::log_summary(LogLevel level){
    if (Log::at_least(level)){
        Log::log(level, "MODEL SUMMARY:");
        int neurons_total = 0; 
        int weights_total=0;      
        for (int l=0;l<layers;l++) {
            std::string layer_type;
            switch (layer[l].type) {
                case POOL_LAYER: layer_type = "pooling"; break;
                case INPUT_LAYER: layer_type = "input"; break;
                case OUTPUT_LAYER: {
                    layer_type = "output";
                    weights_total += layer[l].W_x.get_elements() * layer[l].W_x[0].get_elements();
                } break;
                case LSTM_LAYER: layer_type = "LSTM"; break;
                case RECURRENT_LAYER: layer_type = "recurrent"; break;
                case DENSE_LAYER: {
                    layer_type = "dense";
                    weights_total+=layer[l].W_x.get_elements() * layer[l].W_x[0].get_elements();
                } break;
                case CONVOLUTIONAL_LAYER: layer_type = "convolutional"; break;
                case GRU_LAYER: layer_type = "GRU"; break;
                case DROPOUT_LAYER: layer_type = "dropout(" + std::to_string(layer[l].dropout_ratio) + ")"; break;
                case RELU_LAYER: layer_type = "ReLU"; break;
                case LRELU_LAYER: layer_type = "lReLU"; break;
                case ELU_LAYER: layer_type = "lReLU"; break;
                case SIGMOID_LAYER: layer_type = "sigmoid"; break;
                case TANH_LAYER: layer_type = "tanh"; break;
                case FLATTEN_LAYER: layer_type = "flatten"; break;
                default: layer_type = "unknown layer type"; break;
            }
            Log::log(level, "layer ", l, ": ", layer_type, " ",
                            layer[l].type==DROPOUT_LAYER ? std::to_string(layer[l].dropout_ratio*100)+"%" : "",
                            layer[l].is_stacked ?
                            layer[l].feature_stack_h[0].get_shapestring() + " * " + std::to_string(feature_maps) + " feature maps (=stacked) " :
                            layer[l].h.get_shapestring(),
                            " = ", std::to_string(layer[l].neurons) + " neurons",
                            layer[l].type==DENSE_LAYER || layer[l].type==OUTPUT_LAYER ?
                            ", " + std::to_string(layer[l].W_x.get_elements()*layer[l].W_x[0].get_elements()) + " weights" : "");
            neurons_total += layer[l].neurons;
        }
        Log::log(level, "neurons total: ", neurons_total);
        Log::log(level, "weights total: ", weights_total);
        std::string loss_function_string;
        switch (opt_method){
            case VANILLA: loss_function_string = "vanilla stochastic gradient descent"; break;
            case MOMENTUM: loss_function_string = "stochastic gradient descent with momentum"; break;
            case NESTEROV: loss_function_string = "Nesterov gradient descent"; break;
            case ADAM: loss_function_string = "ADAM optimizer"; break;
            case ADADELTA: loss_function_string = "ADADELTA"; break;
            case RMSPROP: loss_function_string = "RMSprop"; break;
            case ADAGRAD: loss_function_string = "AdaGrad"; break;
            default: loss_function_string = "unknown"; break;
        }
        Log::log(level, "backprop optimization method: ", loss_function_string);
        Log::log(level, "learning rate: ", lr);
    }
}   

// performs a single iteration of backpropagation
void NeuralNet::backpropagate(bool batch_gradient_averaging){
    // - delta rule for output neurons: delta w_ij= lr * loss_func_derivative * act'(net_inp_j) * out_i
    // - delta rule for hidden neurons: delta w_ij=lr * SUM_k[err_k*w_jk] * act'(net_inp_j) * out_i
    // - general rule: delta w_ij= lr * partial_error_k * layer_function_drv * out_i     
    backprop_iterations++;
    // get average gradients for the output layer for this batch
    if (batch_gradient_averaging){
        layer[layers-1].gradient/=sample_counter;
    }
    // update weights and push partial derivative to preceding layer
    for (int l=layers-1; l>0; l--){
        // clip gradient to set abs max
        if (gradient_clipping){
            layer[l].gradient = layer[l].gradient.min(max_gradient);
            layer[l].gradient = layer[l].gradient.max(-max_gradient);
        }        
        // set vanishing gradients (i.e. NAN) to a very small random number
        layer[l].gradient += (layer[l].gradient != layer[l].gradient) * rnd<double>::uniform(1.0e-12,-1.0e-12);

        switch (layer[l].type){

            case POOL_LAYER: {
                if (layer[l-1].is_stacked){
                    layer[l].gradient_stack = layer[l].gradient.dissect(layer[l].dimensions-1);
                    // initialize variables
                    std::vector<int> index_i(layer[l-1].dimensions);
                    std::vector<int> index_j(layer[l].dimensions);
                    std::vector<int> combined_index(layer[l-1].dimensions);
                    Array<int> slider_box(layer[l].pooling_slider_shape);
                    std::vector<int> box_element_index(slider_box.get_dimensions());
                    // iterate over feature maps                 
                    for (int map=0; map<feature_maps; map++){
                        // reset gradients for current maps to all zeros
                        layer[l-1].gradient_stack[map].fill_zeros();
                        // iterate over pooled elements
                        for (int j=0;j<layer[l].neurons;j++){
                            // get associated index (this layer)
                            index_j = layer[l].feature_stack_h.get_index(j);
                            // get corresponding source index (layer l-1)
                            for (int n=0; n<layer[l-1].dimensions; n++){
                                index_i[n] = index_j[n] * layer[l].pooling_stride_shape[n];
                            }
                            // iterate over slider box elements
                            for (int s=0; s<slider_box.get_elements(); s++) {
                                // get combined index for element within slider box
                                box_element_index = slider_box.get_index(s);
                                for (int n=0; n<layer[l-1].dimensions;n++){
                                    combined_index[n] = index_i[n] + box_element_index[n];
                                }
                                // (1) for MEAN pooling
                                if (layer[l].pooling_method == PoolMethod::MEAN){
                                    double fraction = layer[l-1].feature_stack_h[map].get(combined_index) / (layer[l].feature_stack_h[map][j] * slider_box.get_elements());
                                    layer[l-1].gradient_stack[map].set(combined_index, fraction * layer[l].gradient_stack[map][j]);
                                }
                                // (2) all other methods
                                else {
                                    if (layer[l-1].feature_stack_h[map].get(combined_index) == layer[l].feature_stack_h[map][j]){
                                        layer[l-1].gradient_stack[map].set(combined_index,layer[l].gradient_stack[map][j]);
                                    }
                                }
                            }
                        }                        
                    }
                    layer[l-1].gradient=layer[l-1].gradient_stack.stack();
                }
                else {
                    layer[l-1].gradient.fill_zeros();
                    // initialize variables
                    std::vector<int> index_i(layer[l-1].dimensions);
                    std::vector<int> index_j(layer[l].dimensions);
                    std::vector<int> combined_index(layer[l-1].dimensions);
                    Array<int> slider_box(layer[l].pooling_slider_shape);
                    std::vector<int> box_element_index(slider_box.get_dimensions());   
                    // reset gradients to all zeros
                    layer[l-1].gradient.fill_zeros();                 
                    // iterate over pooled elements
                    for (int j=0;j<layer[l].neurons;j++){
                        // get associated index (this layer)
                        index_j = layer[l].h.get_index(j);
                        // get corresponding source index (layer l-1)
                        for (int n=0; n<layer[l-1].dimensions; n++){
                            index_i[n] = index_j[n] * layer[l].pooling_stride_shape[n];
                        }
                        // iterate over slider box elements
                        for (int s=0; s<slider_box.get_elements(); s++) {
                            // get combined index for element within slider box
                            box_element_index = slider_box.get_index(s);
                            for (int n=0; n<layer[l-1].dimensions;n++){
                                combined_index[n] = index_i[n] + box_element_index[n];
                            }
                            // (1) for MEAN pooling
                            if (layer[l].pooling_method == PoolMethod::MEAN){
                                double fraction = layer[l-1].h.get(combined_index) / (layer[l].h[j] * slider_box.get_elements());
                                layer[l-1].gradient.set(combined_index, fraction * layer[l].gradient[j]);
                            }
                            // (2) all other methods
                            else {
                                if (layer[l-1].h.get(combined_index) == layer[l].h[j]){
                                    layer[l-1].gradient.set(combined_index,layer[l].gradient[j]);
                                }
                            }
                        }
                    }
                }
            } break;

            case OUTPUT_LAYER:
            case DENSE_LAYER: {
                // push partial gradients to preceding layer (only if this is not the input layer)
                if (l>1){
                    for (int i=0; i<layer[l-1].h.get_elements(); i++){
                        static Array<double> weights_i;
                        weights_i = *(layer[l-1].W_out[i]);
                        layer[l-1].gradient[i] = weights_i.dotproduct(layer[l].gradient);
                    }
                    // dissect result if preceding layer is stacked
                    if (layer[l-1].is_stacked){
                        layer[l-1].gradient_stack = layer[l-1].gradient.dissect(layer[l-1].dimensions-1);
                    }
                }
                // update weights
                switch (opt_method){
                    case MOMENTUM: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // general rule: delta w_ij= out_i * partial_error_k * layer_function_drv * lr
                            layer[l].delta_W_x[j] = (layer[l].delta_W_x[j] * momentum) + 
                                                    (layer[l-1].h * (layer[l].gradient[j] * lr * (1-momentum)));
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break; 

                    case NESTEROV: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // lookahead step
                            Array<double> lookahead(layer[l].h.get_shape());
                            lookahead = (layer[l].delta_W_x[j] * momentum) +
                                        (layer[l-1].h * (layer[l].gradient[j] * lr * (1-momentum)));
                            // momentum step
                            layer[l].delta_W_x[j] = (lookahead * momentum) +
                                                (layer[l-1].h * (layer[l].gradient[j] * lr * (1-momentum)));
                            // update step
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break;

                    case RMSPROP: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // opt_v update
                            layer[l].opt_v[j] = (layer[l].opt_v[j] * momentum) + (layer[l-1].h * (pow(layer[l].gradient[j],2) * (1-momentum)));
                            // get delta
                            layer[l].delta_W_x[j] = layer[l].opt_v[j].sqrt() * ((1/lr) * layer[l].gradient[j]);
                            // update step
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break;

                    case ADAM: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // opt_v update
                            layer[l].opt_v[j] = (layer[l].opt_v[j] * 0.9) + layer[l-1].h * (layer[l].gradient[j] * 0.1);
                            // opt_w update
                            layer[l].opt_w[j] = (layer[l].opt_w[j] * 0.999) + (layer[l-1].h * (pow(layer[l].gradient[j],2) * 0.001));
                            // get delta
                            layer[l].delta_W_x[j] = layer[l].opt_v[j].Hadamard_division((layer[l].opt_w[j]+1e-8).sqrt()) * lr;
                            // update step
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break;

                    case ADAGRAD: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // opt_v update
                            layer[l].opt_v[j] = layer[l].opt_v[j] + (layer[l-1].h * layer[l].gradient[j]).pow(2);
                            // get delta
                            layer[l].delta_W_x[j] = (layer[l].opt_v[j]+1e-8).sqrt() * (1/lr) * (layer[l-1].h * layer[l].gradient[j]);
                            // update
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break;    

                    case ADADELTA: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // opt_v update
                            layer[l].opt_v[j] = (layer[l].opt_v[j] * 0.9) + ((layer[l-1].h * layer[l].gradient[j]).pow(2) * 0.1);
                            // opt_w update
                            layer[l].opt_w[j] = (layer[l].opt_w[j].pow(2) * 0.9) + (layer[l].delta_W_x[j].pow(2) * 0.1);
                            // get delta
                            layer[l].delta_W_x[j] = (layer[l].opt_w[j]+1e-8).Hadamard_division((layer[l].opt_v[j]+1e-8).sqrt()) * (layer[l-1].h * layer[l].gradient[j]);
                            // update step
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break; 

                    case VANILLA:
                    default: {
                        for (int j=0; j<layer[l].neurons; j++){
                            // general rule: delta w_ij= out_i * partial_error_k * layer_function_drv * lr
                            layer[l].delta_W_x[j] = layer[l-1].h * (layer[l].gradient[j] * lr);
                            layer[l].W_x[j] -= layer[l].delta_W_x[j];
                        }
                    } break;                                                                                               
                }
                // update bias weights
                layer[l].b -= layer[l].gradient * lr;
            } break;

            case LSTM_LAYER: {
                layer[l].gradient_t.pop_first();
                layer[l].gradient_t.push_back(layer[l].gradient);
                layer[l-1].gradient.fill_zeros();
                for (int t = layer[l].timesteps - 2; t >= std::max(layer[l].timesteps - forward_iterations, 1); t--){
                    // Compute the derivative of the loss function with respect to the LSTM output gate activation at the current time step:
                    // dL/do(t) = dL/dy(t) * tanh(c(t)) * sigmoid_derivative(o(t))
                    Array<double> o_gate_gradient = layer[l].gradient_t[t].Hadamard_product(layer[l].c_t[t].activation(ActFunc::TANH)).Hadamard_product(layer[l].o_gate_t[t].derivative(ActFunc::SIGMOID));

                    // Compute the derivative of the loss function with respect to the LSTM cell state at the current time step:
                    // dL/dc(t) = dL/dy(t) * o(t) * tanh_derivative(c(t)) + dL/dc(t+1) * f(t+1)
                    Array<double> c_gate_gradient;
                    if (t < layer[l].timesteps - 1) {
                        c_gate_gradient = layer[l].gradient_t[t].Hadamard_product(layer[l].o_gate_t[t]).Hadamard_product(layer[l].c_gate_t[t].derivative(ActFunc::TANH))
                                        + layer[l].c_t[t+1].Hadamard_product(layer[l].f_gate_t[t+1]);
                    } else {
                        c_gate_gradient = layer[l].gradient_t[t].Hadamard_product(layer[l].o_gate_t[t]).Hadamard_product(layer[l].c_gate_t[t].derivative(ActFunc::TANH));
                    }

                    // Compute the derivative of the loss function with respect to the LSTM forget gate activation at the current time step:
                    // dL/df(t) = dL/dc(t) * c(t-1) * sigmoid_derivative(f(t))
                    Array<double> f_gate_gradient = c_gate_gradient.Hadamard_product(layer[l].c_t[t-1]).Hadamard_product(layer[l].f_gate_t[t].derivative(ActFunc::SIGMOID));

                    // Compute the derivative of the loss function with respect to the LSTM input gate activation at the current time step:
                    // dL/di(t) = dL/dc(t) * g(t) * sigmoid_derivative(i(t))
                    Array<double> i_gate_gradient = c_gate_gradient.Hadamard_product(layer[l].c_gate_t[t]).Hadamard_product(layer[l].i_gate_t[t].derivative(ActFunc::SIGMOID));

                    // Compute the derivative of the loss function with respect to the LSTM candidate activation at the current time step:
                    // dL/dg(t) = dL/dc(t) * i(t) * tanh_derivative(g(t))
                    Array<double> gradient = c_gate_gradient.Hadamard_product(layer[l].i_gate_t[t]).Hadamard_product(layer[l].c_t[t].derivative(ActFunc::TANH));


                    // Compute the gradients of the weight matrices and bias vectors for the current time step and update the weights:
                    for (int j = 0; j < layer[l].neurons; j++) {
                        // dL/dW_i = dL/di(t) * x(t)^T
                        layer[l].W_i[j] -= layer[l].x_t[t] * i_gate_gradient[j] * lr;

                        // dL/dW_f = dL/df(t) * x(t)^T
                        layer[l].W_f[j] -= layer[l].x_t[t] * f_gate_gradient[j] * lr;

                        // dL/dW_o = dL/do(t) * x(t)^T
                        layer[l].W_o[j] -= layer[l].x_t[t] * o_gate_gradient[j] * lr;

                        // dL/dW_c = dL/dg(t) * x(t)^T
                        layer[l].W_c[j] -= layer[l].x_t[t] * gradient[j] * lr;

                        // dL/dU_i = dL/di(t) * h(t-1)^T
                        layer[l].U_i[j] -= layer[l].h_t[t-1] * i_gate_gradient[j] * lr;

                        // dL/dU_f = dL/df(t) * h(t-1)^T
                        layer[l].U_f[j] -= layer[l].h_t[t-1] * f_gate_gradient[j] * lr;

                        // dL/dU_o = dL/do(t) * h(t-1)^T
                        layer[l].U_o[j] -= layer[l].h_t[t-1] * o_gate_gradient[j] * lr;

                        // dL/dU_c = dL/dg(t) * h(t-1)^T
                        layer[l].U_c[j] -= layer[l].h_t[t-1] * gradient[j] * lr;

                        // dL/db_i = dL/di(t)
                        layer[l].b_i[j] -= i_gate_gradient[j] * lr;

                        // dL/db_f = dL/df(t)
                        layer[l].b_f[j] -= f_gate_gradient[j] * lr;

                        // dL/db_o = dL/do(t)
                        layer[l].b_o[j] -= o_gate_gradient[j] * lr;

                        // dL/db_c = dL/dg(t)
                        layer[l].b_c[j] -= gradient[j] * lr;
                    }

                    // Compute the gradient propagated to the previous layer
                    for (int i=0; i<layer[l-1].neurons; i++){
                        layer[l-1].gradient += (layer[l].U_i[i].tensordot(i_gate_gradient, {0})
                                            + layer[l].U_f[i].tensordot(f_gate_gradient, {0})
                                            + layer[l].U_o[i].tensordot(o_gate_gradient, {0})
                                            + layer[l].U_c[i].tensordot(gradient, {0}));
                    }
                }

                // Update the hidden state of the LSTM layer
                layer[l].h = layer[l].h_t[layer[l].timesteps - 1];

                // dissect result if preceding layer is stacked
                if (layer[l-1].is_stacked){
                    layer[l-1].gradient_stack = layer[l-1].gradient.dissect(layer[l-1].dimensions-1);
                }
            } break;

            case RECURRENT_LAYER: {
                // push partial gradients to preceding layer
                layer[l-1].gradient.fill_zeros();
                for (int i=0; i<layer[l-1].h.get_elements(); i++){
                    Array<double> weights_i = *(layer[l-1].W_out[i]);
                    layer[l-1].gradient[i] = weights_i.dotproduct(layer[l].gradient);
                }
                // dissect result if preceding layer is stacked
                if (layer[l-1].is_stacked){
                    layer[l-1].gradient_stack = layer[l-1].gradient.dissect(layer[l-1].dimensions-1);
                }                
                // initialize t as current timestep
                int t = layer[l].timesteps-1;
                // roll gradients by 1 timestep, the update the most recent gradients
                layer[l].gradient_t.pop_first();
                layer[l].gradient_t.push_back(layer[l].gradient);
                // iterate backwards through time
                for (; t>0; t--){
                    for (int j=0; j<layer[l].neurons;j++){
                        // push partial gradients to previous timestep
                        layer[l].gradient_t[t-1][j] += layer[l].U[j].dotproduct(layer[l].gradient_t[t]);
                        // update W_x weights                            
                        layer[l].W_x[j] -= layer[l-1].h * layer[l].gradient_t[t][j] * lr;
                        // update U weights
                        layer[l].U[j] -= layer[l].h_t[t-1] * layer[l].gradient_t[t][j] * lr;
                        // update bias
                        layer[l].b.data[j] -= layer[l].gradient_t[t][j] * lr;
                    }
                }
            } break;

            case CONVOLUTIONAL_LAYER: {
                for (int map=0; map<feature_maps; map++){
                    layer[l-1].gradient_stack[map] = layer[l].gradient_stack[map].convolution(layer[l].filter_stack[map].flip(), true);
                    if (layer[l-1].is_stacked){
                        if (layer[l].padding){
                            Array<double> inputs_copy = layer[l-1].feature_stack_h[map];
                            for (int d=0; d<(int)layer[l].filter_shape.size(); d++){
                                inputs_copy.grow(layer[l].filter_shape[d]-1,d);
                            }
                            layer[l].filter_stack[map] -= inputs_copy.convolution(layer[l].gradient_stack[map], false) * lr;
                        }
                        else {
                            layer[l].filter_stack[map] -= layer[l-1].feature_stack_h[map].convolution(layer[l].gradient_stack[map], false) * lr;
                        }
                    }
                    else {
                        if (layer[l].padding){
                            Array<double> inputs_copy = layer[l-1].h;
                            for (int d=0; d<(int)layer[l].filter_shape.size(); d++){
                                inputs_copy.grow(layer[l].filter_shape[d]-1,d);
                            }
                            layer[l].filter_stack[map] -= inputs_copy.convolution(layer[l].gradient_stack[map], false) * lr;
                        }
                        else {
                            layer[l].filter_stack[map] -= layer[l-1].h.convolution(layer[l].gradient_stack[map], false) * lr;
                        }
                    }
                }
                if (!layer[l-1].is_stacked){
                    layer[l-1].gradient = layer[l-1].gradient_stack.stack();
                }
            } break;

            case GRU_LAYER: {
                int t = layer[l].timesteps - 1;

                // Compute the gradient of the loss function with respect to the GRU hidden state at the last timestep
                layer[l].gradient_t[t] = layer[l].gradient;

                // Backpropagate the gradient through time
                for (; t >= 0; t--) {
                    // Compute the gradients of the weight matrices for the current time step and update the weights
                    for (int j = 0; j < layer[l].neurons; j++) {

                        // Compute dL/dW_z = dL/dh_t * dz_gate_t * x_t
                        layer[l].W_z[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].x_t[t]).Hadamard_product(layer[l].z_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;
                        
                        // Compute dL/dW_r = dL/dh_t * dr_gate_t * x_t
                        layer[l].W_r[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].x_t[t]).Hadamard_product(layer[l].r_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;

                        // Compute dL/dW_c = dL/dh_t * dc_gate_t * x_t
                        layer[l].W_c[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].x_t[t]).Hadamard_product(layer[l].c_gate_t[t].derivative(ActFunc::TANH)) * lr;
                        
                        // Compute dL/dU_z = dL/dh_t * dz_gate_t * h_t-1
                        layer[l].U_z[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].h_t[t-1]).Hadamard_product(layer[l].z_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;

                        // Compute dL/dU_r = dL/dh_t * dr_gate_t * h_t-1
                        layer[l].U_r[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].h_t[t-1]).Hadamard_product(layer[l].r_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;

                        // Compute dL/dU_c = dL/dh_t * dc_gate_t * (r_gate_t * h_t-1)
                        layer[l].U_c[j] -= layer[l].gradient_t[t].Hadamard_product(layer[l].h_t[t-1].Hadamard_product(layer[l].r_gate_t[t]) * layer[l].c_gate_t[t].derivative(ActFunc::TANH)) * lr;
                    }

                    // update bias vectors:

                    // Compute dL/db_z = dL/dh_t * dz_gate_t
                    layer[l].b_z -= layer[l].gradient_t[t].Hadamard_product(layer[l].z_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;  

                    // Compute dL/db_r = dL/dh_t * dr_gate_t
                    layer[l].b_r -= layer[l].gradient_t[t].Hadamard_product(layer[l].r_gate_t[t].derivative(ActFunc::SIGMOID)) * lr;   

                    // Compute dL/db_c = dL/dh_t * dc_gate_t
                    layer[l].b_c -= layer[l].gradient_t[t].Hadamard_product(layer[l].c_gate_t[t].derivative(ActFunc::TANH)) * lr;                                                                           

                    // Compute the gradient propagated to the previous GRU layer
                    layer[l-1].gradient.fill_zeros();
                    for (int i=0; i<layer[l-1].neurons;i++){
                        layer[l-1].gradient += (layer[l].U_z[i].tensordot(layer[l].gradient_t[t].Hadamard_product(layer[l].z_gate_t[t].derivative(ActFunc::SIGMOID)))
                                            + layer[l].U_r[i].tensordot(layer[l].gradient_t[t].Hadamard_product(layer[l].r_gate_t[t].derivative(ActFunc::SIGMOID)))
                                            + layer[l].U_c[i].tensordot(layer[l].gradient_t[t].Hadamard_product(layer[l].c_gate_t[t].derivative(ActFunc::TANH))));
                    }
                }
                
                // dissect result if preceding layer is stacked
                if (layer[l-1].is_stacked){
                    layer[l-1].gradient_stack = layer[l-1].gradient.dissect(layer[l-1].dimensions-1);
                }
            } break;

            case DROPOUT_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l].gradient_stack[map];
                        layer[l-1].gradient_stack[map] *= layer[l].gradient_stack[map] && layer[l].feature_stack_h[map];
                    }
                }
                else {
                    layer[l-1].gradient = layer[l].gradient;
                    layer[l-1].gradient *= layer[l].gradient && layer[l].h;
                }
            } break;

            case RELU_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l-1].gradient_stack[map].derivative(ActFunc::RELU).Hadamard_product(layer[l].gradient_stack[map]);
                    }
                }
                else {
                    layer[l-1].gradient = layer[l-1].h.derivative(ActFunc::RELU).Hadamard_product(layer[l].gradient);
                }
            } break;

            case LRELU_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l-1].gradient_stack[map].derivative(ActFunc::LRELU).Hadamard_product(layer[l].gradient_stack[map]);
                    }
                }
                else {
                    layer[l-1].gradient = layer[l-1].h.derivative(ActFunc::LRELU).Hadamard_product(layer[l].gradient);
                }
            } break;

            case ELU_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l-1].gradient_stack[map].derivative(ActFunc::ELU).Hadamard_product(layer[l].gradient_stack[map]);
                    }
                }
                else {
                    layer[l-1].gradient = layer[l-1].h.derivative(ActFunc::ELU).Hadamard_product(layer[l].gradient);
                }
            } break;

            case SIGMOID_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l-1].gradient_stack[map].derivative(ActFunc::SIGMOID).Hadamard_product(layer[l].gradient_stack[map]);
                    }
                }
                else {
                    layer[l-1].gradient = layer[l-1].h.derivative(ActFunc::SIGMOID).Hadamard_product(layer[l].gradient);
                }
            } break;

            case TANH_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    for (int map=0; map<feature_maps; map++){
                        layer[l-1].gradient_stack[map] = layer[l-1].gradient_stack[map].derivative(ActFunc::TANH).Hadamard_product(layer[l].gradient_stack[map]);
                    }
                }
                else {
                    layer[l-1].gradient = layer[l-1].h.derivative(ActFunc::TANH).Hadamard_product(layer[l].gradient);
                }
            } break;

            case FLATTEN_LAYER: {
                // push gradients to preceding layer
                if (layer[l-1].is_stacked){
                    int j=0;
                    for (int map=0; map<feature_maps; map++){
                        for (int i=0;i<layer[l-1].gradient_stack[map].get_elements(); i++, j++){
                            layer[l-1].gradient_stack[map].set(i, layer[l].gradient[j]);
                        }
                    }
                }
                else {
                    for (int i=0; i<layer[l-1].gradient.get_elements(); i++){
                        layer[l-1].gradient.set(i,layer[l].gradient[i]);
                    }
                }
            } break;

            default:
                layer[l-1].gradient = layer[l].gradient;
                break;
        }  
        if (Log::at_least(LOG_LEVEL_DEBUG)){
            layer[l].gradient.print("gradient["+std::to_string(l)+"]:", " ");
        }
    }
    // reset output layer gradient
    layer[layers-1].gradient.fill_zeros();
    // reset sample counter
    sample_counter = 0;    
}

// calculates the loss according to the specified loss function;
// note: forward pass and label-assignments must be done first!
void NeuralNet::calculate_loss(){
    // get index of output layer
    int l=layers-1;
    sample_counter++;
    loss_counter++;

    switch (loss_function){
        // Mean Squared Error
        case MSE: {
            layer[l].loss_sum += (layer[l].label - layer[l].h).pow(2);
            layer[l].gradient += (layer[l].h - layer[l].label) * 2;
        } break;

        // Mean Absolute Error
        case MAE: {
            layer[l].loss_sum += (layer[l].label - layer[l].h).abs();
            layer[l].gradient += (layer[l].h - layer[l].label).sign();
        } break;

        // Mean Absolute Percentage Error
        case MAPE: {
            layer[l].loss_sum += (layer[l].label - layer[l].h).Hadamard_division(layer[l].label).abs();
            layer[l].gradient += (layer[l].h - layer[l].label).sign().Hadamard_product(layer[l].label).Hadamard_division((layer[l].h - layer[l].label)).abs();
        } break;

        // Mean Squared Logarithmic Error
        case MSLE: {              
            layer[l].loss_sum += ((layer[l].h + 1).log() - (layer[l].label + 1).log()).pow(2);
            layer[l].gradient += (((layer[l].h + 1).log() - (layer[l].label + 1).log()).Hadamard_division(layer[l].h + 1)) * 2;
        } break;

        // Categorical Crossentropy
        case CAT_CROSS_ENTR: {
            // because the label is one-hot encoded, only the output that is labeled as true gets the update
            int true_output = 0;
            for (int j=0;j<layer[l].neurons;j++){
                if (layer[l].h[j]) {
                    true_output = j;
                }
            }
            layer[l].loss_sum[true_output] -= layer[l].label.Hadamard_product(layer[l].h.log()).sum();
            layer[l].gradient -= layer[l].label.Hadamard_division(layer[l].h);
        } break;

        // Sparse Categorical Crossentropy
        case SPARSE_CAT_CROSS_ENTR: {
            layer[l].loss_sum -= layer[l].h.log();
            layer[l].gradient -= layer[l].label.Hadamard_division(layer[l].h);
        } break;

        // Binary Crossentropy
        case BIN_CROSS_ENTR: {
            layer[l].loss_sum -= layer[l].label.Hadamard_product(layer[l].h.log()) + ((layer[l].label*-1)+1) * ((layer[l].h*-1)+1).log();
            layer[l].gradient -= layer[l].label.Hadamard_division(layer[l].h) - ((layer[l].label*-1)+1).Hadamard_division((layer[l].h*-1)+1);
        } break;

        // Kullback-Leibler Divergence
        case KLD: {
            layer[l].loss_sum += layer[l].label.Hadamard_product(layer[l].label.Hadamard_division(layer[l].h).log());
            layer[l].gradient += layer[l].label.Hadamard_product(layer[l].label.log() - layer[l].h.log() + 1);
        } break;

        // Poisson
        case POISSON: {
            layer[l].loss_sum += layer[l].h - layer[l].label.Hadamard_product(layer[l].h.log());
            layer[l].gradient += layer[l].h - layer[l].label;
        } break;

        // Hinge
        case HINGE: {
            layer[l].loss_sum += ((layer[l].label.Hadamard_product(layer[l].h) * -1) + 1).max(0);
            layer[l].gradient -= layer[l].label.Hadamard_product((layer[l].label.Hadamard_product(layer[l].h)*-1 + 1).sign());
        } break;

        // Squared Hinge
        case SQUARED_HINGE: {
            layer[l].loss_sum += ((layer[l].label.Hadamard_product(layer[l].h) * -1 + 1).max(0)).pow(2);
            layer[l].gradient += (layer[l].label.Hadamard_product(layer[l].h)*-1 + 1).max(0) * 2;
        } break;

        default: {
            // do nothing
        } break;
    }
    layer[l].loss = layer[l].loss_sum / loss_counter;
    loss_avg = layer[l].loss.sum() / layer[l].neurons;
}

// creates a new input layer or adds a new parallel input shape
// to a preexisting input layer
void NeuralNet::addlayer_input(std::vector<int> shape){
    layer_init(INPUT_LAYER, shape);  
}

// creates a new output layer or adds a new parallel output shape
// to a preexisting output layer
void NeuralNet::addlayer_output(std::vector<int> shape, LossFunction loss_function){
    layer_init(OUTPUT_LAYER, shape);
    int l = layers-1;
    layer[l].label = Array<double>(shape);
    layer[l].loss = Array<double>(shape);
    layer[l].loss_sum = Array<double>(shape);
    loss_function = loss_function;
    layer_make_dense_connections();
    layer[l].opt_v=Array<Array<double>>(layer[l].shape);
    layer[l].opt_w=Array<Array<double>>(layer[l].shape);
    for (int j=0;j<layer[l].neurons;j++){
        layer[l].opt_v[j] = Array<double>(layer[l-1].h.get_shape());
        layer[l].opt_w[j] = Array<double>(layer[l-1].h.get_shape());
    }
}

// creates an LSTM layer of the specified shape
void NeuralNet::addlayer_lstm(std::vector<int> shape, const int timesteps){
    layer_init(LSTM_LAYER, shape);
    int l = layers-1;
    layer[l].timesteps = timesteps;
    layer[l].c_t = Array<Array<double>>({timesteps});
    layer[l].h_t = Array<Array<double>>({timesteps});
    layer[l].x_t = Array<Array<double>>({timesteps});
    layer[l].gradient_t = Array<Array<double>>({timesteps});
    layer[l].f_gate_t = Array<Array<double>>({timesteps});
    layer[l].i_gate_t = Array<Array<double>>({timesteps});
    layer[l].o_gate_t = Array<Array<double>>({timesteps});
    layer[l].c_gate_t = Array<Array<double>>({timesteps});
    for (int t=0;t<timesteps;t++){
        // initialize vectors of timesteps for the cell state, hidden state and gradient h_t
        layer[l].c_t[t] = Array<double>(shape); layer[l].c_t[t].fill_zeros();
        layer[l].h_t[t] = Array<double>(shape); layer[l].h_t[t].fill_zeros();
        layer[l].x_t[t] = Array<double>(shape); layer[l].h_t[t].fill_zeros();
        layer[l].gradient_t[t] = Array<double>(shape); layer[l].gradient_t[t].fill_zeros();
        // initialize gate value arrays
        layer[l].f_gate_t[t] = Array<double>(shape); layer[l].f_gate_t[t].fill_zeros();
        layer[l].i_gate_t[t] = Array<double>(shape); layer[l].i_gate_t[t].fill_zeros();
        layer[l].o_gate_t[t] = Array<double>(shape); layer[l].o_gate_t[t].fill_zeros();
        layer[l].c_gate_t[t] = Array<double>(shape); layer[l].c_gate_t[t].fill_zeros();
    }   
    // initialize gate weights to h(t-1)
    layer[l].U_f = Array<Array<double>>(shape);
    layer[l].U_i = Array<Array<double>>(shape);
    layer[l].U_o = Array<Array<double>>(shape);
    layer[l].U_c = Array<Array<double>>(shape);
    // initialize gate weights to x(t)
    layer[l].W_f = Array<Array<double>>(layer[l-1].h.get_shape());
    layer[l].W_i = Array<Array<double>>(layer[l-1].h.get_shape());
    layer[l].W_o = Array<Array<double>>(layer[l-1].h.get_shape());
    layer[l].W_c = Array<Array<double>>(layer[l-1].h.get_shape());     
    // initialize weight matrices
    for (int j=0;j<layer[l].c_t.get_elements();j++){
        layer[l].U_f[j] = Array<double>(shape); layer[l].U_f[j].fill_He_ReLU(layer[l].neurons);
        layer[l].U_i[j] = Array<double>(shape); layer[l].U_i[j].fill_He_ReLU(layer[l].neurons);
        layer[l].U_o[j] = Array<double>(shape); layer[l].U_o[j].fill_He_ReLU(layer[l].neurons);
        layer[l].U_c[j] = Array<double>(shape); layer[l].U_c[j].fill_He_ReLU(layer[l].neurons);
        layer[l].W_f[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_f[j].fill_He_ReLU(layer[l-1].neurons);
        layer[l].W_i[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_i[j].fill_He_ReLU(layer[l-1].neurons);
        layer[l].W_o[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_o[j].fill_He_ReLU(layer[l-1].neurons);
        layer[l].W_c[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_c[j].fill_He_ReLU(layer[l-1].neurons);    
    }
    // initialize biases
    layer[l].b_f = Array<double>(shape); layer[l].b_f.fill_He_ReLU(layer[l].neurons);
    layer[l].b_i = Array<double>(shape); layer[l].b_f.fill_He_ReLU(layer[l].neurons);
    layer[l].b_o = Array<double>(shape); layer[l].b_f.fill_He_ReLU(layer[l].neurons);
    layer[l].b_c = Array<double>(shape); layer[l].b_f.fill_He_ReLU(layer[l].neurons);
}

// creates a GRU layer
void NeuralNet::addlayer_GRU(std::vector<int> shape, const int timesteps){
    layer_init(GRU_LAYER, shape);
    int l = layers-1;
    layer[l].timesteps = timesteps;
    layer[l].c_t = Array<Array<double>>({timesteps});
    layer[l].h_t = Array<Array<double>>({timesteps});
    layer[l].x_t = Array<Array<double>>({timesteps});
    layer[l].z_gate_t = Array<Array<double>>({timesteps});
    layer[l].r_gate_t = Array<Array<double>>({timesteps});
    layer[l].c_gate_t = Array<Array<double>>({timesteps});    
    for (int t=0;t<timesteps;t++){
        // initialize vectors of timesteps for the cell state and hidden state
        layer[l].c_t[t] = Array<double>(shape); layer[l].c_t[t].fill_zeros();
        layer[l].h_t[t] = Array<double>(shape); layer[l].h_t[t].fill_zeros();
        layer[l].x_t[t] = Array<double>(shape); layer[l].h_t[t].fill_zeros();
        // initialize gate value arrays  
        layer[l].z_gate_t[t] = Array<double>(shape); layer[l].z_gate_t[t].fill_zeros();
        layer[l].r_gate_t[t] = Array<double>(shape); layer[l].r_gate_t[t].fill_zeros();
        layer[l].c_gate_t[t] = Array<double>(shape); layer[l].c_gate_t[t].fill_zeros();
    }
    // initialize gate weights to h(t-1) 
    layer[l].U_z = Array<Array<double>>(shape);
    layer[l].U_r = Array<Array<double>>(shape);
    layer[l].U_c = Array<Array<double>>(shape);     
    // initialize gate weights to x(t)
    layer[l].W_z = Array<Array<double>>(layer[l-1].h.get_shape());
    layer[l].W_r = Array<Array<double>>(layer[l-1].h.get_shape());
    layer[l].W_c = Array<Array<double>>(layer[l-1].h.get_shape());
    // initialize weight matrices
    for (int j=0;j<layer[l].neurons;j++){
        layer[l].U_z[j] = Array<double>(shape); layer[l].U_z[j].fill_He_ReLU(layer[l].neurons);
        layer[l].U_r[j] = Array<double>(shape); layer[l].U_r[j].fill_He_ReLU(layer[l].neurons);
        layer[l].U_c[j] = Array<double>(shape); layer[l].U_c[j].fill_He_ReLU(layer[l].neurons);
        layer[l].W_z[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_z[j].fill_He_ReLU(layer[l-1].neurons);
        layer[l].W_r[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_r[j].fill_He_ReLU(layer[l-1].neurons);
        layer[l].W_c[j] = Array<double>(layer[l-1].h.get_shape()); layer[l].W_c[j].fill_He_ReLU(layer[l-1].neurons);
    }
    // initialize biases
    layer[l].b_z = Array<double>(shape); layer[l].b_z.fill_He_ReLU(layer[l].neurons);
    layer[l].b_r = Array<double>(shape); layer[l].b_r.fill_He_ReLU(layer[l].neurons);
    layer[l].b_c = Array<double>(shape); layer[l].b_c.fill_He_ReLU(layer[l].neurons);   
}

// creates a recurrent layer of the specified shape
void NeuralNet::addlayer_recurrent(std::vector<int> shape, int timesteps){
    layer_init(RECURRENT_LAYER, shape);
    int l = layers-1;
    layer[l].timesteps = timesteps;
    layer[l].x_t = Array<Array<double>>({timesteps});
    layer[l].h_t = Array<Array<double>>({timesteps});
    layer[l].gradient_t = Array<Array<double>>({timesteps});
    for (int t=0; t<timesteps; t++){
        layer[l].x_t[t] = Array<double>(shape); layer[l].x_t[t].fill_zeros();
        layer[l].h_t[t] = Array<double>(shape); layer[l].h_t[t].fill_zeros();
        layer[l].gradient_t[t] = Array<double>(shape);
    }
    layer[l].U = Array<Array<double>>(shape);
    for (int j=0; j<layer[l].neurons; j++){
        layer[l].U[j] = Array<double>(shape); layer[l].U[j].fill_He_ReLU(layer[l].neurons);
    }
}

// creates a fully connected layer
void NeuralNet::addlayer_dense(std::vector<int> shape){
    layer_init(DENSE_LAYER, shape);
    layer_make_dense_connections();
    int l=layers-1;
    layer[l].opt_v=Array<Array<double>>(layer[l].shape);
    layer[l].opt_w=Array<Array<double>>(layer[l].shape);
    for (int j=0;j<layer[l].neurons;j++){
        layer[l].opt_v[j] = Array<double>(layer[l-1].h.get_shape());
        layer[l].opt_w[j] = Array<double>(layer[l-1].h.get_shape());
    }    
}

// creates a convolutional layer
void NeuralNet::addlayer_convolutional(const int filter_radius, bool padding){
    // check valid layer type
    if (layers==0){
        Log::log(LOG_LEVEL_WARNING,
            "invalid usage of method 'void NeuralNet::addlayer_convolutional(const int filter_radius, bool padding)'",
            "the first layer always has to be of type 'INPUT'");
        return;
    }
    // initialize filters
    if (filter_radius<1){
        Log::log(LogLevel::LOG_LEVEL_INFO,
            "the filter radius for CNN layers should be >=1 but is ", filter_radius,
            ", -> will be set to 1");
    }
    // calculate filter shape
    std::vector<int> filter_shape(layer[layers-1].dimensions);
    for (int d=0;d<layer[layers-1].dimensions;d++){
        filter_shape[d] = d<=1 ? 1+2*std::max(1,filter_radius) : layer[layers-1].h.get_shape()[d];
    }
    // initialize layer
    if (layer[layers-1].is_stacked){
        std::vector<int> shape = layer[layers-1].feature_stack_h[0].get_convolution_shape(filter_shape, padding);
        layer_init(CONVOLUTIONAL_LAYER, shape);
    }
    else {  
        std::vector<int> shape = layer[layers-1].h.get_convolution_shape(filter_shape, padding);
        layer_init(CONVOLUTIONAL_LAYER, shape);
    }
    layer[layers-1].padding = padding;
    layer[layers-1].filter_shape = filter_shape;
}

// creates a dropout layer
void NeuralNet::addlayer_dropout(const double ratio){
    layer_init(DROPOUT_LAYER, layer[layers-1].shape);
    layer[layers-1].dropout_ratio = ratio;
}

void NeuralNet::addlayer_sigmoid(){
    layer_init(SIGMOID_LAYER, layer[layers-1].shape);
}

void NeuralNet::addlayer_ReLU(){
    layer_init(RELU_LAYER, layer[layers-1].shape);;   
}

void NeuralNet::addlayer_lReLU(){
    layer_init(LRELU_LAYER, layer[layers-1].shape);  
}

void NeuralNet::addlayer_ELU(){
    layer_init(ELU_LAYER, layer[layers-1].shape);   
}

void NeuralNet::addlayer_tanh(){
    layer_init(TANH_LAYER, layer[layers-1].shape);   
}

void NeuralNet::addlayer_flatten(){
    layer_init(FLATTEN_LAYER, {layer[layers-1].neurons});
}

void NeuralNet::addlayer_pool(PoolMethod method, std::vector<int> slider_shape, std::vector<int> stride_shape){
    std::vector<int> layer_shape(layer[layers-1].dimensions);
    int l=layers;
    if (layer[l-1].is_stacked)
    {
        for (int d=0;d<layer[layers-1].dimensions;d++){
            layer_shape[d] = layer[layers-1].feature_stack_h[0].get_shape()[d] / std::max(1,stride_shape[d]);
        }
    }
    else {
        for (int d=0;d<layer[layers-1].dimensions;d++){
            layer_shape[d] = layer[l-1].h.get_shape()[d] / std::max(1,stride_shape[d]);
        }
    }    
    layer_init(LayerType::POOL_LAYER, layer_shape);
    layer[l].pooling_slider_shape=slider_shape;
    layer[l].pooling_stride_shape=stride_shape;
    layer[l].pooling_method = method;
}

// helper method to make dense connections from
// preceding layer (l-1) to current layer (l)
// and initialize dense connection weights and biases
void NeuralNet::layer_make_dense_connections(){
    int l = layers-1;
    // initialize 'h' Array of preceding stacked layer
    if (layer[l-1].is_stacked){
        layer[l-1].h = Array<double>(layer[l-1].feature_stack_h.get_stacked_shape());
    }     
    // create incoming weights
    layer[l].W_x = Array<Array<double>>(layer[l].shape);
    layer[l].delta_W_x = Array<Array<double>>(layer[l].shape);
    int neurons_i = layer[l-1].h.get_elements();
    int neurons_j = layer[l].neurons;
    for (int j=0;j<neurons_j;j++){
        layer[l].W_x[j] = Array<double>(layer[l-1].h.get_shape());
        layer[l].W_x[j].fill_He_ReLU(neurons_i);
        layer[l].delta_W_x[j] = Array<double>(layer[l-1].h.get_shape());
        layer[l].delta_W_x[j].fill_zeros();
    }
    // attach outgoing weights of preceding layer
    layer[l-1].W_out = Array<Array<double*>>(layer[l-1].h.get_shape());
    for (int i=0;i<neurons_i;i++){
        layer[l-1].W_out[i] = Array<double*>(layer[l].shape);
        for (int j=0;j<neurons_j;j++){
            // store references to associated weights into <double *>
            layer[l-1].W_out[i][j] = &layer[l].W_x[j][i];
            // example of accessing the 'fan out' of weight values for any given neuron 'i' by dereferencing:
            // Array<double> dereferenced = *layer[l-1].W_out[i];
        }
    }
    // initialize bias weights
    layer[l].b = Array<double>(layer[l].shape);
    layer[l].b.fill_He_ReLU(neurons_i);
}

// basic layer setup
void NeuralNet::layer_init(LayerType type, std::vector<int> shape){
    // check valid layer type
    if (layers==0 && type != INPUT_LAYER){
        throw std::invalid_argument("the first layer always has to be of type 'INPUT'");
    }
    if (layers>0 && type == INPUT_LAYER){
        throw std::invalid_argument("input layer already exists");
    }    
    if (layers>0 && layer[layers-1].type == OUTPUT_LAYER){
        throw std::invalid_argument("an output layer has already been defined; can't add any new layers on top");
    }
    if (static_cast<int>(type) < 0 || static_cast<int>(type) >= static_cast<int>(LayerType::LAYER_TYPE_COUNT)){
        throw std::invalid_argument("layer type enum value must be a cardinal that's less than LAYER_TYPE_COUNT");
    }
    // create new layer
    layer.emplace_back(Layer());
    // setup layer parameters
    layers++;
    int l = layers - 1;
    layer[l].type = type;
    layer[l].shape = shape;
    // default: set as not stacked
    layer[l].is_stacked = false;
    // explicitly exclude layers that can't be stacked:
    if (type!=INPUT_LAYER &&
        type!=OUTPUT_LAYER &&
        type!=DENSE_LAYER &&
        type!=LSTM_LAYER &&
        type!=GRU_LAYER &&
        type!=RECURRENT_LAYER &&
        type!=FLATTEN_LAYER){
            // all other layer types inherit stacked status from predecessor
            layer[l].is_stacked = layer[l-1].is_stacked;
        }
    // set convolutional layers as always stacked
    if (type==CONVOLUTIONAL_LAYER){
        layer[l].is_stacked=true;
        layer[l-1].gradient_stack = Array<Array<double>>({feature_maps});
        for (int i=0; i<feature_maps; i++){
            layer[l-1].gradient_stack[i] = Array<double>(shape);
        }
    }
    // initialize 'x', 'h' and 'gradient' arrays
    if (layer[l].is_stacked){
        layer[l].feature_stack_h = Array<Array<double>>({feature_maps});
        layer[l].gradient_stack = Array<Array<double>>({feature_maps});
        for (int i=0; i<feature_maps; i++){
            layer[l].feature_stack_h.data[i] = Array<double>(shape);
            layer[l].gradient_stack.data[i] = Array<double>(shape);
        }
        layer[l].neurons = layer[l].feature_stack_h[0].get_elements() * feature_maps;
    }
    else {
        if (type == FLATTEN_LAYER){
            layer[l].h = Array<double>({layer[l-1].neurons});
            layer[l].gradient = Array<double>({layer[l-1].neurons});
            layer[l].gradient.fill_zeros();
            layer[l].neurons = layer[l-1].neurons;
        }
        else {
            layer[l].h = Array<double>(shape);
            layer[l].gradient = Array<double>(shape);
            layer[l].gradient.fill_zeros();
            layer[l].neurons = layer[l].h.get_elements();
            // if the preceding layer is stacked (while the current one isn't):
            // create combined 'h' array as interface
            if (l>=1 && layer[l-1].is_stacked){
                layer[l-1].h = Array<double>(layer[l-1].feature_stack_h.get_stacked_shape());
                layer[l-1].gradient = Array<double>(layer[l-1].gradient_stack.get_stacked_shape());
            }
        }
    }
    layer[l].dimensions = shape.size();    
}

// scale labels
void NeuralNet::scale_labels(const Array<double>& raw_labels){
    int l=layers-1;
    if (scaling_method==ScalingMethod::MIN_MAX_NORM){
        layer[l].label = (raw_labels-labels_min).Hadamard_division(labels_max-labels_min);
    }
    if (scaling_method==ScalingMethod::MEAN_NORM){
        layer[l].label = (raw_labels-labels_mean).Hadamard_division(labels_max-labels_min);
    }
    if (scaling_method==ScalingMethod::STANDARDIZED){
        layer[l].label = (raw_labels-labels_mean).Hadamard_division(labels_stddev);
    }
}