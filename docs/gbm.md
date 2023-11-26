[[return to main page]](../../../README.md)
## Gradient Boosting Machines Algorithm
Gradient Boosting Machines (GBM) is a popular machine learning algorithm used for both regression and classification tasks. It is an iterative ensemble method that combines multiple weak learners to create a strong predictive model.

Usage: `#include <gbm.h>` or include as part of `<classification.h>` or `<datascience.h>`.
Make sure to keep the include path updated or use relative paths instead.

Dependencies: `<vector>`, `<cmath>`, `<iostream>`;

Constructor: `GBM(int numTrees, int maxDepth, T learningRate);`

Public Methods:
```
void train(const std::vector<std::vector<T>>& X, const std::vector<T>& y);
std::vector<T> predict(const std::vector<std::vector<T>>& X) const;
```
