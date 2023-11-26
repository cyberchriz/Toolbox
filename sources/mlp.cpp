#include "../headers/mlp.h"

// constructor
MLP::MLP(std::string filename){
    this->filename=filename;
    srand(int(time(nullptr)));
    //if (filename!=""){load(filename);}
}

// destructor
MLP::~MLP(){}

// add new network layer
// return value: current number of total layers
int MLP::add_layer(int neurons, OPTIMIZATION_METHOD _opt_method, ACTIVATION_FUNC _activation){
    int inputs_per_neuron = layers==0 ? 0 : layer[layers-1].neurons;
    layer.push_back(Layer(neurons,inputs_per_neuron,_opt_method,_activation));
    layers++;
    MLP::reset_weights();
    return layers;
}

// initialize weights:
// automatically sets appropriate method for a given activation function
void MLP::reset_weights(uint start_layer, uint end_layer, double factor){
    for (int l=fmax(1,start_layer);l<=fmin(layers-1,end_layer);l++){
        int fan_in = 1;
        int fan_out = l<layers-1 ? layer[l+1].neurons : 1;
        if (l>=1){
            fan_in=layer[l-1].neurons; // neurons from preceding layer
            fan_in+=5; // account for bias weight and recurrent weights
            if (l<layers-1){ // =if not last layer
                fan_out=layer[l].neurons;
            }
        }
        ACTIVATION_FUNC act_func = layer[l].activation;
        switch (act_func){
            case f_ReLU:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_He_ReLU(fan_in)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m1_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m2_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m3_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m4_weight=f_He_ReLU(fan_in)*factor;
                }
                break;
            case f_LReLU:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_He_ReLU(fan_in)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m1_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m2_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m3_weight=f_He_ReLU(fan_in)*factor;
                    layer[l].neuron[j].m4_weight=f_He_ReLU(fan_in)*factor;
                }
                break;                
            case f_tanh:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_Xavier_uniform(fan_in,fan_out)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_Xavier_uniform(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m1_weight=f_Xavier_uniform(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m2_weight=f_Xavier_uniform(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m3_weight=f_Xavier_uniform(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m4_weight=f_Xavier_uniform(fan_in,fan_out)*factor;
                }          
                break;
            case f_sigmoid:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m1_weight=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m2_weight=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m3_weight=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m4_weight=f_Xavier_sigmoid(fan_in,fan_out)*factor;
                }              
                break;
            case f_ELU:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_He_ELU(fan_in)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_He_ELU(fan_in)*factor;
                    layer[l].neuron[j].m1_weight=f_He_ELU(fan_in)*factor;
                    layer[l].neuron[j].m2_weight=f_He_ELU(fan_in)*factor;
                    layer[l].neuron[j].m3_weight=f_He_ELU(fan_in)*factor;
                    layer[l].neuron[j].m4_weight=f_He_ELU(fan_in)*factor;
                }          
                break;
            default:
                for (int j=0;j<layer[l].neurons;j++){
                    for (int i=0;i<layer[l-1].neurons;i++){
                        layer[l].neuron[j].input_weight[i]=f_Xavier_normal(fan_in,fan_out)*factor;
                        layer[l].neuron[j].input_weight_delta[i]=0;
                    }
                    layer[l].neuron[j].bias_weight=f_Xavier_normal(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m1_weight=f_Xavier_normal(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m2_weight=f_Xavier_normal(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m3_weight=f_Xavier_normal(fan_in,fan_out)*factor;
                    layer[l].neuron[j].m4_weight=f_Xavier_normal(fan_in,fan_out)*factor;
                }                
        }
        // reinitialize other hyperparameters
        for (int j=0;j<layer[l].neurons;j++){
            for (int i=0;i<layer[l-1].neurons;i++){
                layer[l].neuron[j].opt_v[i]=0;
                layer[l].neuron[j].opt_w[i]=0;
            }
            layer[l].neuron[j].delta_b=0;
            layer[l].neuron[j].delta_m1=0;
            layer[l].neuron[j].delta_m2=0;
            layer[l].neuron[j].delta_m3=0;
            layer[l].neuron[j].delta_m4=0;
        }        
    }
}

// forward propagation
void MLP::feedforward(uint start_layer, uint end_layer){
    // cycle through layers
    for (int l=fmax(1,start_layer);l<=fmin(layers-1,end_layer);l++){
        for (int j=0;j<layer[l].neurons;j++){
            // update recurrent values from last iteration
            if (recurrent && !layer[l].neuron[j].dropout){
                layer[l].neuron[j].m1=layer[l].neuron[j].h;
                layer[l].neuron[j].m2=0.9*layer[l].neuron[j].m2+0.1*layer[l].neuron[j].h;
                layer[l].neuron[j].m3=0.99*layer[l].neuron[j].m3+0.01*layer[l].neuron[j].h;
                layer[l].neuron[j].m4=0.999*layer[l].neuron[j].m4+0.001*layer[l].neuron[j].h;
            }
            // set dropout
            layer[l].neuron[j].dropout=false;
            if (training_mode && l!=0 && l!=layers-1){
                layer[l].neuron[j].dropout = Random<double>::uniform(0,1)<dropout;
                if (layer[l].neuron[j].dropout){
                    continue;
                }
            }
            // get sum of weighted inputs
            layer[l].neuron[j].x=0;            
            for (int i=0;i<layer[l-1].neurons;i++){
                if (!layer[l-1].neuron[i].dropout){
                    layer[l].neuron[j].x+=layer[l-1].neuron[i].h*layer[l].neuron[j].input_weight[i];
                }
            }
            // add weighted bias
            layer[l].neuron[j].x+=1.0*layer[l].neuron[j].bias_weight;
            // add weighted recurrent neurons
            if (recurrent){
                layer[l].neuron[j].x+=layer[l].neuron[j].m1*layer[l].neuron[j].m1_weight;
                layer[l].neuron[j].x+=layer[l].neuron[j].m2*layer[l].neuron[j].m2_weight;
                layer[l].neuron[j].x+=layer[l].neuron[j].m3*layer[l].neuron[j].m3_weight;
                layer[l].neuron[j].x+=layer[l].neuron[j].m4*layer[l].neuron[j].m4_weight;
            }
            // add dropout compensation in training mode
            if (training_mode && l>1){
                layer[l].neuron[j].x *= 1/(1-dropout);
            }            
            // activate
            layer[l].neuron[j].h = activate(layer[l].neuron[j].x,layer[l].activation);

            // reset weights of current layer in case of invalid numbers
            if (std::isnan(layer[l].neuron[j].x) || std::isnan(layer[l].neuron[j].h) ||
                std::isinf(layer[l].neuron[j].x) || std::isinf(layer[l].neuron[j].h)){
                std::cout << "iteration " << backprop_iterations << ": NaN/Inf error in layer " << l << ", neuron " << j
                << " (x=" << layer[l].neuron[j].x << ", h=" << layer[l].neuron[j].h
                << "); resetting weights for this layer\n";
                layer[l].neuron[j].x=0;
                layer[l].neuron[j].h=0;
                reset_weights(l,l);
                // auto-adjust learning rate
                if (lr_auto){
                    lr_adjust_factor *=lr_adjust_fraction;
                }
            }
        }        
        // rescale outputs
        if (l==layers-1){
            for (int j=0;j<layer[l].neurons;j++){
                switch (scaling_method){
                    case none:
                        layer[l].neuron[j].output = layer[l].neuron[j].h;
                        break;
                    case maxabs:
                        layer[l].neuron[j].output = layer[l].neuron[j].h * layer[l].neuron[j].label_maxabs;
                        break;
                    case normalized:
                        layer[l].neuron[j].output = layer[l].neuron[j].h * (layer[l].neuron[j].label_max - layer[l].neuron[j].label_min) + layer[l].neuron[j].label_min;
                        break;
                    default:
                        // =standardized
                        layer[l].neuron[j].output = layer[l].neuron[j].h * layer[l].neuron[j].label_stddev + layer[l].neuron[j].label_rolling_average;
                        break;
                }
            }
        }        
    }
}

// backpropagation
void MLP::backpropagate(){
    if (!training_mode){return;}
    backprop_iterations++;
    // apply learning rate decay
    lr=base_lr * std::pow(0.5,(double(backprop_iterations))/lr_decay);
    lr *= lr_adjust_factor;

    // (I) cycle backwards through layers
    for (int l=layers-1;l>=1;l--){
        // get global errors from output layer
        if (l==layers-1){
            for (int j=0;j<layer[l].neurons;j++){
                // derivative of the 0.5err^2 loss function: scaled_label-output
                static double last_gradient;
                last_gradient = layer[l].neuron[j].gradient;
                layer[l].neuron[j].gradient = layer[l].neuron[j].scaled_label - layer[l].neuron[j].h;

                // gradient NaN/Inf check
                if (std::isnan(layer[l].neuron[j].gradient) || std::isinf(layer[l].neuron[j].gradient)){
                    layer[l].neuron[j].gradient=0;
                    reset_weights(l,l);
                    if (lr_auto){lr_adjust_factor*=lr_adjust_fraction;}
                    break;
                }
                // 0.5err^2 loss
                layer[l].neuron[j].loss = 0.5 * layer[l].neuron[j].gradient * layer[l].neuron[j].gradient;

                // cumulative loss (per neuron)
                if (!std::isnan(layer[l].neuron[j].loss) && !std::isinf(layer[l].neuron[j].loss)){
                    layer[l].neuron[j].loss_sum = layer[l].neuron[j].loss_sum + layer[l].neuron[j].loss;
                }

                // auto-adjust learning rate
                if (lr_auto){
                    lr_adjust_factor = std::fabs(layer[l].neuron[j].gradient) > std::fabs(last_gradient) ?
                            std::fmax(0.0001, lr_adjust_factor*lr_adjust_fraction) :
                            std::fmin(1.0,lr_adjust_factor*inv_fraction);
                    lr=std::fmin(lr,base_lr);
                }
                // gradient clipping
                if (gradient_clipping){
                    layer[l].neuron[j].gradient = fmin(layer[l].neuron[j].gradient, gradient_clipping_threshold);
                    layer[l].neuron[j].gradient = fmax(layer[l].neuron[j].gradient, -gradient_clipping_threshold);
                }                
            }
        }
        // get hidden gradient = SUM_k[err_k*w_jk]
        else{
            for (int j=0;j<layer[l].neurons;j++){
                layer[l].neuron[j].gradient=0;
                if (layer[l].neuron[j].dropout){continue;}
                int fan_out=layer[l+1].neurons;
                for (int k=0;k<fan_out;k++){
                    layer[l].neuron[j].gradient+=layer[l+1].neuron[k].gradient*layer[l+1].neuron[k].input_weight[j];
                }
                if (std::isnan(layer[l].neuron[j].gradient) || std::isinf(layer[l].neuron[j].gradient)){
                    layer[l].neuron[j].gradient=0;
                    reset_weights(l,l);
                    break;
                }
            }
        }
    }
    // (II) cycle through layers a second time in order to update the weights
    // (=with respect to how much they each influenced the (hidden) error)
    // - delta rule for output neurons: delta w_ij=lr*(label-output)*act'(net_inp_j)*out_i
    // - delta rule for hidden neurons: delta w_ij=lr*SUM_k[err_k*w_jk]*act'(net_inp_j)*out_i
    // - general rule: delta w_ij=lr*error_k*act'(net_inp_j)*out_i    
    for (int l=layers-1;l>=1;l--){
        int input_neurons=layer[l-1].neurons;
        OPTIMIZATION_METHOD method=layer[l].opt_method;
        for (int j=0;j<layer[l].neurons;j++){
            if (layer[l].neuron[j].dropout){continue;}
            for (int i=0;i<input_neurons;i++){
                if (layer[l-1].neuron[i].dropout){continue;}
                if (method==VANILLA){
                    // get delta
                    layer[l].neuron[j].input_weight_delta[i] = lr * layer[l].neuron[j].gradient * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l-1].neuron[i].h;
                    // update
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i]+layer[l].neuron[j].input_weight_delta[i];
                }
                if (method==MOMENTUM){
                    // get delta
                    layer[l].neuron[j].input_weight_delta[i] = (lr_momentum*layer[l].neuron[j].input_weight_delta[i]) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l-1].neuron[i].h;
                    // update
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i]+layer[l].neuron[j].input_weight_delta[i];
                }                
                else if (method==NESTEROV){
                    // lookahead step
                    double lookahead = (lr_momentum*layer[l].neuron[j].input_weight_delta[i]) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l-1].neuron[i].h;
                    // momentum step
                    layer[l].neuron[j].input_weight_delta[i] = (lr_momentum*lookahead) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l-1].neuron[i].h;
                    // update step
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i] + layer[l].neuron[j].input_weight_delta[i];
                }
                else if (method==RMSPROP){
                    // opt_v update
                    layer[l].neuron[j].opt_v[i] =  lr_momentum*layer[l].neuron[j].opt_v[i] + (1-lr_momentum)*pow(deactivate(layer[l].neuron[j].x,layer[l].activation),2) * layer[l].neuron[j].gradient;
                    // get delta
                    layer[l].neuron[j].input_weight_delta[i] =  lr / (sqrt(layer[l].neuron[j].opt_v[i]+1e-8)+__DBL_MIN__) * pow(layer[l].neuron[j].h,2) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h;
                    // update
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i]+layer[l].neuron[j].input_weight_delta[i];
                }
                else if (method==ADADELTA){
                    // opt_v update
                    layer[l].neuron[j].opt_v[i] =  opt_beta1 * layer[l].neuron[j].opt_v[i] + (1-opt_beta1) * pow(deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h,2);
                    // opt_w update
                    layer[l].neuron[j].opt_w[i] =  (opt_beta1 * pow(layer[l].neuron[j].opt_w[i],2)) + (1-opt_beta1)*pow(layer[l].neuron[j].input_weight_delta[i],2);
                    // get delta
                    layer[l].neuron[j].input_weight_delta[i] =  sqrt(layer[l].neuron[j].opt_w[i]+1e-8)/(sqrt(layer[l].neuron[j].opt_v[i]+1e-8)+__DBL_MIN__) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h;
                    // update
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i]+layer[l].neuron[j].input_weight_delta[i];
                }
                else if (method==ADAM){ // =ADAM without minibatch
                    // opt_v update
                    layer[l].neuron[j].opt_v[i] =  opt_beta1 * layer[l].neuron[j].opt_v[i] + (1-opt_beta1) * deactivate(layer[l].neuron[j].x, layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h;
                    // opt_w update
                    layer[l].neuron[j].opt_w[i] = opt_beta2 * layer[l].neuron[j].opt_w[i] * pow(deactivate(layer[l].neuron[j].x, layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h,2);
                    // get delta
                    double v_t = layer[l].neuron[j].opt_v[i]/(1-opt_beta1);
                    double w_t = layer[l].neuron[j].opt_w[i]/(1-opt_beta2);
                    layer[l].neuron[j].input_weight_delta[i] =  lr * (v_t/(sqrt(w_t+1e-8))+__DBL_MIN__);
                    // update
                    layer[l].neuron[j].input_weight[i] =  layer[l].neuron[j].input_weight[i]  + layer[l].neuron[j].input_weight_delta[i];
                }
                else if (method==ADAGRAD){
                    // opt_v update
                    layer[l].neuron[j].opt_v[i] =  layer[l].neuron[j].opt_v[i] + pow(deactivate(layer[l].neuron[j].x, layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h,2);
                    // get delta
                    layer[l].neuron[j].input_weight_delta[i] = lr / sqrt(layer[l].neuron[j].opt_v[i] +1e-8) * deactivate(layer[l].neuron[j].x, layer[l].activation) * layer[l].neuron[j].gradient * layer[l-1].neuron[i].h;
                    // update
                    layer[l].neuron[j].input_weight[i] = layer[l].neuron[j].input_weight[i]  + layer[l].neuron[j].input_weight_delta[i];
                }                
                // NaN/Inf check
                if (std::isnan(layer[l].neuron[j].input_weight[i]) || std::isinf(layer[l].neuron[j].input_weight[i])){
                    reset_weights(l,l);
                    break;
                }
            }
            // update bias weights (Vanilla)
            layer[l].neuron[j].delta_b = (lr_momentum*layer[l].neuron[j].delta_b) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation);
            layer[l].neuron[j].bias_weight = layer[l].neuron[j].bias_weight + layer[l].neuron[j].delta_b;
            // update recurrent weights (Vanilla)
            if (recurrent){
                layer[l].neuron[j].delta_m1 = (lr_momentum*layer[l].neuron[j].delta_m1) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].m1;
                layer[l].neuron[j].delta_m2 = (lr_momentum*layer[l].neuron[j].delta_m2) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].m2;
                layer[l].neuron[j].delta_m3 = (lr_momentum*layer[l].neuron[j].delta_m3) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].m3;
                layer[l].neuron[j].delta_m4 = (lr_momentum*layer[l].neuron[j].delta_m4) + (1-lr_momentum)*(lr*layer[l].neuron[j].gradient) * deactivate(layer[l].neuron[j].x,layer[l].activation) * layer[l].neuron[j].m4;
                layer[l].neuron[j].m1_weight = layer[l].neuron[j].m1_weight + layer[l].neuron[j].delta_m1;
                layer[l].neuron[j].m2_weight = layer[l].neuron[j].m2_weight + layer[l].neuron[j].delta_m2;
                layer[l].neuron[j].m3_weight = layer[l].neuron[j].m3_weight + layer[l].neuron[j].delta_m3;
                layer[l].neuron[j].m4_weight = layer[l].neuron[j].m4_weight + layer[l].neuron[j].delta_m4;
            }
        }
    }                
}

// set a single input value via index (with auto-scaling)
void MLP::set_input(uint index, double value){
    layer[0].neuron[index].x = value;
    layer[0].neuron[index].input_min = fmin(value,layer[0].neuron[index].input_min);
    layer[0].neuron[index].input_max = fmax(value,layer[0].neuron[index].input_max);
    layer[0].neuron[index].input_maxabs = fmax(layer[0].neuron[index].input_maxabs,abs(value));
    if (backprop_iterations<10){
        layer[0].neuron[index].input_rolling_average = layer[0].neuron[index].input_rolling_average*0.5+value*0.5;
        layer[0].neuron[index].input_mdev2 = pow((value-layer[0].neuron[index].input_rolling_average),2);
        layer[0].neuron[index].input_variance = layer[0].neuron[index].input_variance*0.5+layer[0].neuron[index].input_mdev2*0.5;     
    }
    else if (backprop_iterations<200){
        layer[0].neuron[index].input_rolling_average = layer[0].neuron[index].input_rolling_average*0.95+value*0.05;
        layer[0].neuron[index].input_mdev2 = pow((value-layer[0].neuron[index].input_rolling_average),2);
        layer[0].neuron[index].input_variance = layer[0].neuron[index].input_variance*0.95+layer[0].neuron[index].input_mdev2*0.05;     
    }
    else{
        layer[0].neuron[index].input_rolling_average = layer[0].neuron[index].input_rolling_average*0.999+value*0.001;
        layer[0].neuron[index].input_mdev2 = pow((value-layer[0].neuron[index].input_rolling_average),2);
        layer[0].neuron[index].input_variance = layer[0].neuron[index].input_variance*0.999+layer[0].neuron[index].input_mdev2*0.001;         
    } 
    layer[0].neuron[index].input_stddev = sqrt(layer[0].neuron[index].input_variance);

    switch (scaling_method){
        case none:
            layer[0].neuron[index].h = value;
            break;
        case maxabs:
            // -1 to 1
            layer[0].neuron[index].h = value / (layer[0].neuron[index].input_maxabs + __DBL_EPSILON__);
            break;
        case normalized:
            // 0 to 1
            layer[0].neuron[index].h = value - layer[0].neuron[index].input_min;
            layer[0].neuron[index].h = layer[0].neuron[index].h / (layer[0].neuron[index].input_max - layer[0].neuron[index].input_min + __DBL_EPSILON__);
            break;
        default:
        // case standardized: // (µ=0, sigma=1)
            layer[0].neuron[index].h = layer[0].neuron[index].h - layer[0].neuron[index].input_rolling_average; 
            layer[0].neuron[index].h = layer[0].neuron[index].h / (layer[0].neuron[index].input_stddev + __DBL_EPSILON__);
            break;
    }
}

// get single output via 1d index
double MLP::get_output(uint index){
    return layer[layers-1].neuron[index].output;
}
   
// get a single 'h' from a hidden layer via 1d index (e.g. for autoencoder bottleneck)      
double MLP::get_hidden(uint index,uint layer_index){
    return layer[layer_index].neuron[index].h;
}

// set a single label value via 1-dimensional index (with auto-scaling)
void MLP::set_label(uint index, double value){
    layer[layers-1].neuron[index].x = value;
    layer[layers-1].neuron[index].label_min = fmin(value,layer[layers-1].neuron[index].label_min);
    layer[layers-1].neuron[index].label_max = fmax(value,layer[layers-1].neuron[index].label_max);
    layer[layers-1].neuron[index].label_maxabs = fmax(layer[layers-1].neuron[index].label_maxabs,abs(value));
    if (backprop_iterations<10){
        layer[layers-1].neuron[index].label_rolling_average = layer[layers-1].neuron[index].label_rolling_average*0.5+value*0.5;
        layer[layers-1].neuron[index].label_mdev2 = pow((value-layer[layers-1].neuron[index].label_rolling_average),2);
        layer[layers-1].neuron[index].label_variance = layer[layers-1].neuron[index].label_variance*0.5+layer[layers-1].neuron[index].label_mdev2*0.5;
    }
    else if (backprop_iterations<200){
        layer[layers-1].neuron[index].label_rolling_average = layer[layers-1].neuron[index].label_rolling_average*0.95+value*0.05;
        layer[layers-1].neuron[index].label_mdev2 = pow((value-layer[layers-1].neuron[index].label_rolling_average),2);
        layer[layers-1].neuron[index].label_variance = layer[layers-1].neuron[index].label_variance*0.95+layer[layers-1].neuron[index].label_mdev2*0.05;
    }
    else{
        layer[layers-1].neuron[index].label_rolling_average = layer[layers-1].neuron[index].label_rolling_average*0.999+value*0.001;
        layer[layers-1].neuron[index].label_mdev2 = pow((value-layer[layers-1].neuron[index].label_rolling_average),2);
        layer[layers-1].neuron[index].label_variance = layer[layers-1].neuron[index].label_variance*0.999+layer[layers-1].neuron[index].label_mdev2*0.001;
    }
    layer[layers-1].neuron[index].label_stddev = sqrt(layer[layers-1].neuron[index].label_variance);    

    switch (scaling_method){
        case none:
            layer[layers-1].neuron[index].scaled_label = value;
            break;
        case maxabs:
            // -1 to 1
            layer[layers-1].neuron[index].scaled_label = value / (layer[layers-1].neuron[index].label_maxabs + __DBL_EPSILON__);
            break;
        case normalized:
            // 0 to 1
            layer[layers-1].neuron[index].scaled_label = value - layer[layers-1].neuron[index].label_min;
            layer[layers-1].neuron[index].scaled_label = layer[layers-1].neuron[index].h / (layer[layers-1].neuron[index].label_max - layer[layers-1].neuron[index].label_min + __DBL_EPSILON__);
            break;
        default:
            // standardized (µ=0, sigma=1)
            layer[layers-1].neuron[index].scaled_label = layer[layers-1].neuron[index].h - layer[layers-1].neuron[index].label_rolling_average; 
            layer[layers-1].neuron[index].scaled_label = layer[layers-1].neuron[index].h / (layer[layers-1].neuron[index].label_stddev + __DBL_EPSILON__);
            break;
    }
}

// autoencode (set inputs as labels)
void MLP::autoencode(){
    if (layer[0].neurons!=layer[layers-1].neurons){return;}
    static int items=layer[0].neurons;
    for (int j=0;j<items;j++){
        set_label(j,layer[0].neuron[j].x);
    }
}

// get average loss per output neuron across all backprop iterations
double MLP::get_loss_avg(){
    double result=0;
    if (backprop_iterations==0){return result;}
    int n=layer[layers-1].neurons;
    for (int j=0;j<n;j++){
        result+=layer[layers-1].neuron[j].loss_sum/backprop_iterations;
    }
    result/=n;
    return result;
}

// get average 'h'
//      (=result of activation function)
//      (=output without rescaling)
double MLP::get_avg_h(){
    double result=0;
    int n=layer[layers-1].neurons;
    for (int j=0;j<n;j++){
        result+=layer[layers-1].neuron[j].h;
    }
    result/=n;
    return result;
}

// get average output (=result after rescaling of 'h')
double MLP::get_avg_output(){
    double result=0;
    int n=layer[layers-1].neurons;
    for (int j=0;j<n;j++){
        result+=layer[layers-1].neuron[j].output;
    }
    result/=n;
    return result;
}

// save network data into file
void MLP::save(std::string filename) {

}