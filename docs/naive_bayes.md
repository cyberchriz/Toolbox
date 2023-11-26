[[return to main page]](../../../README.md)
## Naive Bayes Algorithm
Naive Bayes is a probabilistic machine learning algorithm that uses Bayes' theorem to make predictions by assuming that the features are conditionally independent given the class.

Bayes' theorem is a fundamental concept in probability theory that describes the probability of an event occurring based on prior knowledge or information. 
The formula states that the probability of A given B is equal to the probability of B given A multiplied by the probability of A, divided by the probability of B.

Usage: `#include <naive_bayes.h>` or include as part of `<classification.h>` or `<datascience.h>`;

Dependencies: `<vector>`, `<unordered_map>`, `<cmath>`;

Constructor: `NaiveBayes()`

Public Methods:
```
// Add a training example
void addExample(const std::vector<T>& features, const std::string& label);

// Train the model
void train();

// Classify a new example
std::string classify(const std::vector<T>& features);
```