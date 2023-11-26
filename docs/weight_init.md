[[return to main page]](../../../../README.md)
## Neural Network Weight Initialization

Usage: `#include <activation_functions.h>` or include as part of `<datascience.h>`.
Make sure to update the include path or use relative paths instead.

Available Functions:
```
// normal "Xavier" weight initialization (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_normal(int fan_in, int fan_out);

// uniform "Xavier" weight initializiation (by Xavier Glorot & Bengio) for tanh activation
double f_Xavier_uniform(int fan_in, int fan_out);

// uniform "Xavier" weight initialization for sigmoid activation
double f_Xavier_sigmoid(int fan_in, int fan_out);

// "Kaiming He" normal weight initialization, used for ReLU activation
double f_He_ReLU(int fan_in);

// modified "Kaiming He" nornal weight initialization, used for ELU activation
double f_He_ELU(int fan_in);
```