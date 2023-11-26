[[return to main page]](../../../README.md)
## Decision Tree Algorithm
The Decision Tree Algorithm is a supervised machine learning algorithm that is used for both classification and regression tasks. It builds a tree-like model of decisions and their possible consequences based on the input data. Each internal node of the tree represents a decision based on a feature of the data, and each leaf node represents a class label or a numerical value. The goal is to create a tree that accurately predicts the target variable for new data based on the decisions made in the tree. The algorithm is popular due to its simplicity, interpretability, and ability to handle both categorical and numerical data.

Usage: `#include <decisiontree.h>` or include as part of `<classification.h>` or `<datascience.h>`;

Dependencies: `<vector>`, `<cmath>`, `<algorithm>`;

Constructor: `DecisionTree()`;

Public Methods:

- Fit the decision tree to the training data X with the corresponding labels y:
```
void fit(const std::vector<std::vector<T>>& X, const std::vector<T>& y);
```

- Predict the class labels for the test data:
```
std::vector<T> predict(const std::vector<std::vector<T>>& X) const;
```