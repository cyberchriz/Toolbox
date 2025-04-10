// author: Christian Suer (github.com/cyberchriz/git)

// The code implements a decision tree classifier using the ID3 algorithm for splitting nodes.

// Preprocessor directives

#ifndef DECISION_TREE_H
#define DECISION_TREE_H
#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

// Main class
template <typename T>
class DecisionTree {
public:
    // Constructor
    DecisionTree();

    // Destructor
    ~DecisionTree();

    // Fit the decision tree to the training data X with the corresponding labels y
    void fit(const std::vector<std::vector<T>>& X, const std::vector<T>& y);

    // Predict the class labels for the test data
    std::vector<T> predict(const std::vector<std::vector<T>>& X) const;

private:
    struct Node {
        bool is_leaf;
        T label;
        int feature_index;
        T split_value;
        Node* left_child;
        Node* right_child;
    };

    Node* root;

    // Recursive function to build the decision tree
    // (using the training data X and the corresponding labels y)
    Node* build_tree(const std::vector<std::vector<T>>& X, const std::vector<T>& y);

    // Recursive function to traverse the decision tree and predict the labels for the test data X
    void traverse_tree(const Node* node, const std::vector<std::vector<T>>& X, std::vector<T>& predictions) const;

    // Calculate the entropy of the target variable y
    // entropy = - sum(p * log2(p))
    // where p is the proportion of samples in each class
    double calculate_entropy(const std::vector<T>& y) const;

    // Find the feature that maximizes the information gain and the corresponding split value
    void find_best_split(const std::vector<std::vector<T>>& X, const std::vector<T>& y, int& best_feature, T& best_split_value) const;

    // Split the dataset based on the best feature and split value
    std::pair<std::vector<std::vector<T>>, std::vector<T>> split_dataset(const std::vector<std::vector<T>>& X, const std::vector<T>& y, int feature_index, T split_value) const;

    std::vector<T> get_unique_values(const std::vector<T>& v) const;
};




//+------------------------------------------------------------------+
//|                          DEFINITIONS                             |
//+------------------------------------------------------------------+

template <typename T>
DecisionTree<T>::DecisionTree() : root(nullptr) {}

template <typename T>
DecisionTree<T>::~DecisionTree() {
    // Delete all the nodes in the decision tree
    std::function<void(Node*)> delete_tree = [&](Node* node) {
        if (node != nullptr) {
            delete_tree(node->left_child);
            delete_tree(node->right_child);
            delete node;
        }
        };

    delete_tree(root);
}

template <typename T>
void DecisionTree<T>::fit(const std::vector<std::vector<T>>& X, const std::vector<T>& y) {
    // Build the decision tree
    root = build_tree(X, y);
}

template <typename T>
std::vector<T> DecisionTree<T>::predict(const std::vector<std::vector<T>>& X) const {
    std::vector<T> predictions(X.size());

    // Traverse the decision tree and predict the labels for the test data
    traverse_tree(root, X, predictions);

    return predictions;
}

template <typename T>
typename DecisionTree<T>::Node* DecisionTree<T>::build_tree(const std::vector<std::vector<T>>& X, const std::vector<T>& y) {
    // Base case: if all the labels are the same, return a leaf node with that label
    if (std::adjacent_find(y.begin(), y.end(), std::not_equal_to<>()) == y.end()) {
        Node* leaf = new Node();
        leaf->is_leaf = true;
        leaf->label = y[0];
        leaf->left_child = nullptr;
        leaf->right_child = nullptr;
        return leaf;
    }

    // Find the feature that maximizes the information gain
    int best_feature;
    T best_split_value;
    find_best_split(X, y, best_feature, best_split_value);

    // Split the dataset based on the best feature and split value
    auto [X_left, y_left] = split_dataset(X, y, best_feature, best_split_value);
    auto [X_right, y_right] = split_dataset(X, y, best_feature, best_split_value + 1);

    // Create a new internal node with the best feature and split value
    Node* node = new Node();
    node->is_leaf = false;
    node->feature_index = best_feature;
    node->split_value = best_split_value;

    // Recursively build the left and right subtrees
    node->left_child = build_tree(X_left, y_left);
    node->right_child = build_tree(X_right, y_right);

    return node;
}

template <typename T>
void DecisionTree<T>::traverse_tree(const Node* node, const std::vector<std::vector<T>>& X, std::vector<T>& predictions) const {
    // Base case: if the node is a leaf node, assign its label to all the samples in the corresponding leaf of the decision tree
    if (node->is_leaf) {
        std::fill(predictions.begin(), predictions.end(), node->label);
        return;
    }

    // Recursively traverse the left and right subtrees based on the split criterion
    std::vector<T> X_feature(X.size());
    for (int i = 0; i < X.size(); i++) {
        X_feature[i] = X[i][node->feature_index];
    }

    std::vector<int> left_indices;
    std::vector<int> right_indices;
    for (int i = 0; i < X_feature.size(); i++) {
        if (X_feature[i] < node->split_value) {
            left_indices.push_back(i);
        }
        else {
            right_indices.push_back(i);
        }
    }

    std::vector<T> predictions_left(left_indices.size());
    if (!left_indices.empty()) {
        traverse_tree(node->left_child, { X[left_indices] }, predictions_left);
        for (int i = 0; i < left_indices.size(); i++) {
            predictions[left_indices[i]] = predictions_left[i];
        }
    }

    std::vector<T> predictions_right(right_indices.size());
    if (!right_indices.empty()) {
        traverse_tree(node->right_child, { X[right_indices] }, predictions_right);
        for (int i = 0; i < right_indices.size(); i++) {
            predictions[right_indices[i]] = predictions_right[i];
        }
    }
}

template <typename T>
double DecisionTree<T>::calculate_entropy(const std::vector<T>& y) const {
    int n = y.size();

    // Count the number of samples in each class
    std::map<T, int> class_counts;
    for (const auto& label : y) {
        class_counts[label]++;
    }

    // Calculate the entropy of the target variable
    double entropy = 0.0;
    for (const auto& [label, count] : class_counts) {
        double p = static_cast<double>(count) / n;
        entropy -= p * std::log2(p);
    }

    return entropy;
}

template <typename T>
void DecisionTree<T>::find_best_split(const std::vector<std::vector<T>>& X, const std::vector<T>& y, int& best_feature, T& best_split_value) const {
    double best_gain = -std::numeric_limits<double>::infinity();

    // Calculate the entropy of the parent node
    double parent_entropy = calculate_entropy(y);

    // Iterate over all the features and split values to find the feature that maximizes the information gain
    for (int feature_index = 0; feature_index < X[0].size(); feature_index++) {
        std::vector<T> X_feature(X.size());
        for (int i = 0; i < X.size(); i++) {
            X_feature[i] = X[i][feature_index];
        }

        std::vector<T> unique_values = get_unique_values(X_feature);

        for (const auto& split_value : unique_values) {
            // Split the dataset based on the current feature and split value
            auto [X_left, y_left] = split_dataset(X, y, feature_index, split_value);
            auto [X_right, y_right] = split_dataset(X, y, feature_index, split_value + 1);

            // Calculate the information gain of the split
            double left_weight = static_cast<double>(X_left.size()) / X.size();
            double right_weight = static_cast<double>(X_right.size()) / X.size();
            double gain = parent_entropy - left_weight * calculate_entropy(y_left) - right_weight * calculate_entropy(y_right);

            // Update the best feature and split value if the current split has higher information gain
            if (gain > best_gain) {
                best_gain = gain;
                best_feature = feature_index;
                best_split_value = split_value;
            }
        }
    }
}

template <typename T>
std::pair<std::vector<std::vector<T>>, std::vector<T>> DecisionTree<T>::split_dataset(const std::vector<std::vector<T>>& X, const std::vector<T>& y, int feature_index, T split_value) const {
    std::vector<std::vector<T>> X_left;
    std::vector<T> y_left;
    std::vector<std::vector<T>> X_right;
    std::vector<T> y_right;

    for (int i = 0; i < X.size(); i++) {
        if (X[i][feature_index] < split_value) {
            X_left.push_back(X[i]);
            y_left.push_back(y[i]);
        }
        else {
            X_right.push_back(X[i]);
            y_right.push_back(y[i]);
        }
    }

    return { X_left, y_left, X_right, y_right };
}

template <typename T>
std::vector<T> DecisionTree<T>::get_unique_values(const std::vector<T>& v) const {
    std::vector<T> unique_values;
    std::set<T> unique_values_set;

    for (const auto& value : v) {
        if (!unique_values_set.count(value)) {
            unique_values.push_back(value);
            unique_values_set.insert(value);
        }
    }

    return unique_values;
}

#endif