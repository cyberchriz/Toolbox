[[return to main page]](../../../../README.md)
## Neural Network (type: Multilayer Perceptron)
## `class MLP`
- a class for fully connected neural networks
- full freedom over the number of layers and neurons per layer
- automated scaling of inputs ("features"), outputs and labels ("targets")
- choice among various optimizers

Usage:
- `#include <mlp.h>`or include as part of `datascience.h`
  (make sure to update the compiler's include path or use relative paths instead)
- create a new instance of MLP, e.g.
```
MLP network;
```

Methods:
| method | description |
|--------|-------------|
| `MLP::add_layer(int neurons, OPTIMIZATION_METHOD _opt_method, ACTIVATION_FUNC _activation)`| adds a new layer to an MLP network object, with the specified number of neurons, [optimization method](#optimizers) and [activation function](../../general/docs/activation_functions.md) |
| `MLP::reset_weights(uint start_layer, uint end_layer, double factor)` | re-initializes the weights (the method is automatically optimized for the number of neurons ("fan in", "fan out") and activation function) |
| `MLP::feedforward(uint start_layer, uint end_layer)` | forward propagation through the specified layers (default: entire network); make sure to set inputs first! |
| `MLP::backpropagate()` | adapts the weights according to their contribution to the output errors; make sure to set the labels (=targets) first! |
| `MLP::set_input(uint index, double value)` | assigns a value to the input neuron that has the given index (indices start from 0) |
| `MLP::get_input(uint index, uint l)` | returns a copy of the input value of any neuron (input=default, hidden or output) with the given neuron index and layer index (indices start from 0) |
| `MLP::get_output(uint index)` | returns the rescaled output of the output neuron that has the given index (indices start fom 0) |
| `MLP::get_hidden(uint index,uint layer_index)` | returns the internal state (=result of the activation function) of the specified hidden neuron |
| `MLP::set_label(uint index, double value)` | sets the label (target) of the specified output neuron |
| `MLP::autoencode()` | maps the input values (features) to the outputs as labels (targets) |
| `MLP::save(std::string filename)` | |
| `MLP::network_exists()` | returns false if the network doesn't have any layers yet |
| `MLP::get_loss_avg()` | returns the average loss across all output neurons (should decrease over time) |
| `MLP::set_training_mode(bool active)` | if set to false: disables backpropagation and dropout |
| `MLP::set_learning_rate(double value)` | set to a low value (usually <=0.005); doesn't have any effect with the ADADELTA optimizer |
| `MLP::set_learning_rate_decay(uint value)` | sets a halflife as number of backprop iterations |
| `MLP::set_learning_momentum(double value)` | sets a momentum value (must be <1; e.g. 0.9) to avoid getting stuck in local minima |
| `MLP::set_learning_rate_auto(bool active=true)` | enables automatic learning rate adjustment to avoid exploding gradients |
| `MLP::set_scaling_method(SCALING method=normalized)` | sets the [scaling](#scaling) method for input features, outputs and labels |
| `MLP::set_dropout(double value)` | sets a dropout ratio (e.g. 0.2) to mitigate overfitting |
| `MLP::set_recurrent(bool confirm)` | enables RNN train|
| `MLP::set_gradient_clipping(bool active,double threshold)` | maximizes the gradient in order to prevent overshooting the miminmum |
| `MLP::get_avg_h()` | returns the average raw output, before re-scaling |
| `MLP::get_avg_output()` | returns the average output, after re-scaling |     
| `MLP::get_lr()` | returns the current effective learning rate (with decay and auto-lr) |
___
### Optimizers
These are various algorithms that are designed to make the learning process (weight correction during backpropagation) more efficient and robust. The available optimizers are:
- Vanilla (optional: momentum, auto learning-rate)
- Nesterov
- ADAM
- AdaGrad
- ADADELTA
- RMSprop

### Scaling
Available methods for automatic scaling of input features and labels, as well as output re-scaling
are listed in this enumeration:
```
enum SCALING {
   none,              // no scaling
   standardized,      // standard deviation method (µ=0, sigma=1)
   normalized,        // minmax, range 0 to 1, =default
   maxabs             // minmax, range -1 to +1
};
```

[[return to main page]](../../../../README.md)