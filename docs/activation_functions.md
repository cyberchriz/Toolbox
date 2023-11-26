[[return to main page]](../../../../README.md)

## Activation Functions
Activations functions in neural networks are applied to the sum of weighted inputs of a given neuron.
Their main purpose is to introduce non-linearity.
Examples of performant activation functions are the "rectified linear unit" (f_ReLU) and "leaky rectified linear unit" (f_LReLU);
the latter remains differentiable for negative values.
Among the more common functions are also tanh (f_tanh) and sigmoid (f_sigmoid).

A full list of the available activation functions in this library is given by the following enumeration:
```
enum ACTIVATION_FUNC {
   f_ident,        // identity function
   f_sigmoid,      // sigmoid (logistic)
   f_ELU,          // exponential linear unit (ELU)
   f_ReLU,         // rectified linear unit (ReLU)
   f_LReLU,        // leaky ReLU
   f_tanh,         // hyperbolic tangent (tanh)
   f_oblique_tanh, // oblique tanh (custom)
   f_tanh_rectifier,// tanh rectifier
   f_arctan,       // arcus tangent (arctan)
   f_arsinh,       // area sin. hyperbolicus (inv. hyperbol. sine)
   f_softsign,     // softsign (Elliot)
   f_ISRU,         // inverse square root unit (ISRU)
   f_ISRLU,        // inv.squ.root linear unit (ISRLU)
   f_softplus,     // softplus
   f_bentident,    // bent identity
   f_sinusoid,     // sinusoid
   f_sinc,         // cardinal sine (sinc)
   f_gaussian,     // gaussian normal
   f_differentiable_hardstep, // differentiable hardstep
   f_leaky_diff_hardstep, // leaky differentiable hardstep
   f_softmax,      // normalized exponential (softmax)
   f_oblique_sigmoid, // oblique sigmoid
   f_log_rectifier, // log rectifier
   f_leaky_log_rectifier, // leaky log rectifier
   f_ramp          // ramp
  };
```

Any of the activation functions above can be called with the `activate()` function:
```
double activate(double x, ACTIVATION_FUNC f);
```

Accordingly, in order to obtain the derivative of any of these activation functions, the `deactivate()` function is used:
```
double activate(double x, ACTIVATION_FUNC f);
```

In order to get the full name of any activation function as a std::string return value, we can use this function:
```
std::string actfunct_string(ACTIVATION_FUNC f);
```

[[return to main page]](../../../../README.md)