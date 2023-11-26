[[return to main page]](../../../README.md)
## k nearest neighbors algorithm
The k nearest neighbors (k-NN) algorithm is a machine learning algorithm that is used for classification and regression tasks. It works by finding the k closest data points in a training set to a given input data point, and then using the labels or values of those k neighbors to make a prediction for the input data point. The value of k is a hyperparameter that needs to be chosen beforehand (default=3), and can be tuned to improve the accuracy of the algorithm.

Usage: `#include <k_nearest.h>` or include as part of `<classification.h>` or `<datascience.h>`.

Make sure to update the include path or use relative links instead.

Dependencies: `<iostream>`, `<vector>`, `<algorithm>`, `<cmath>`, `<queue>`;

Parametric constructor: `KNearest(int k = 3)`

Public Methods:
- Add training sample(s) with their corresponding class label(s) via any of these methods:
```
void add_samples(const std::vector<std::vector<T>>& training_samples, const std::vector<int>& labels);
void add_samples(const std::vector<TrainingSample<T>>& training_samples);
void add_samples(const TrainingSample<T>& sample);
void add_samples(const std::vector<T>& sample, T label);
```
- predict a class from a test sample:
```
int predict(const std::vector<T>& test_sample) const;
```