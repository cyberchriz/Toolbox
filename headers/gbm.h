// author: Christian Suer
// github.com/cyberchriz/git/DataScience

// Gradient Boosting Machines (GBM) algorithm

#pragma once
#include <vector>
#include <cmath>
#include <iostream>

template <typename T>
class GBM {
public:
    GBM(int numTrees, int maxDepth, T learningRate);
    void train(const std::vector<std::vector<T>>& X, const std::vector<T>& y);
    std::vector<T> predict(const std::vector<std::vector<T>>& X) const;
    
private:
    struct TreeNode {
        T splitValue;
        int featureIndex;
        TreeNode* leftChild;
        TreeNode* rightChild;
    };
    
    std::vector<TreeNode*> trees_;
    int numTrees_;
    int maxDepth_;
    T learningRate_;
    
    T calculateError(const std::vector<T>& predicted, const std::vector<T>& actual) const;
    TreeNode* buildTree(const std::vector<std::vector<T>>& X, const std::vector<T>& y, const std::vector<int>& sampleIndices, int depth);
    T calculateSplitValue(const std::vector<std::vector<T>>& X, const std::vector<T>& y, const std::vector<int>& sampleIndices, int featureIndex) const;
    void traverseTree(const TreeNode* node, const std::vector<T>& sample, T& prediction) const;
    std::vector<T> predictSubset(const std::vector<std::vector<T>>& X, const std::vector<int>& sampleIndices, int featureIndex, T splitValue, bool leftBranch) const;
};




//+------------------------------------------------------------------+
//|                          DEFINITIONS                             |
//+------------------------------------------------------------------+

template <typename T>
GBM<T>::GBM(int numTrees, int maxDepth, T learningRate) :
    numTrees_(numTrees),
    maxDepth_(maxDepth),
    learningRate_(learningRate) {}

template <typename T>
void GBM<T>::train(const std::vector<std::vector<T>>& X, const std::vector<T>& y) {
    int numSamples = X.size();
    int numFeatures = X[0].size();
    std::vector<T> residuals(numSamples);
    for (int i = 0; i < numTrees_; ++i) {
        // calculate residuals
        std::vector<T> predicted = predict(X);
        for (int j = 0; j < numSamples; ++j) {
            residuals[j] = y[j] - predicted[j];
        }
        // build tree
        std::vector<int> sampleIndices(numSamples);
        for (int j = 0; j < numSamples; ++j) {
            sampleIndices[j] = j;
        }
        trees_.push_back(buildTree(X, residuals, sampleIndices, 0));
    }
}

template <typename T>
std::vector<T> GBM<T>::predict(const std::vector<std::vector<T>>& X) const {
    int numSamples = X.size();
    std::vector<T> predictions(numSamples, 0.0);
    for (int i = 0; i < numTrees_; ++i) {
        for (int j = 0; j < numSamples; ++j) {
            T prediction;
            traverseTree(trees_[i], X[j], prediction);
            predictions[j] += learningRate_ * prediction;
        }
    }
    return predictions;
}

template <typename T>
T GBM<T>::calculateError(const std::vector<T>& predicted, const std::vector<T>& actual) const {
    T error = 0.0;
    int numSamples = predicted.size();
    for (int i = 0; i < numSamples; ++i) {
        T diff = predicted[i] - actual[i];
        error += diff * diff;
    }
    return error / numSamples;
}

template <typename T>
typename GBM<T>::TreeNode* GBM<T>::buildTree(const std::vector<std::vector<T>>& X, const std::vector<T>& y, const std::vector<int>& sampleIndices, int depth) {
    int numSamples = sampleIndices.size();
    int numFeatures = X[0].size();
    // check stopping conditions
    if (depth == maxDepth_ || numSamples == 1) {
        TreeNode* node = new TreeNode();
        node->splitValue = 0.0;
        node->featureIndex = -1;
        node->leftChild = nullptr;
        node->rightChild = nullptr;
        return node;
    }
    T bestError = std::numeric_limits<T>::max();
    int bestFeatureIndex = -1;
    T bestSplitValue = 0.0;

    // try all possible splits
    for (int i = 0; i < numFeatures; ++i) {
        T splitValue = calculateSplitValue(X, y, sampleIndices, i);
        std::vector<int> leftIndices;
        std::vector<int> rightIndices;
        for (int j = 0; j < numSamples; ++j) {
            if (X[sampleIndices[j]][i] <= splitValue) {
                leftIndices.push_back(sampleIndices[j]);
            }
            else {
                rightIndices.push_back(sampleIndices[j]);
            }
        }

        // calculate error
        std::vector<T> leftY(leftIndices.size());
        std::vector<T> rightY(rightIndices.size());
        for (int j = 0; j < leftIndices.size(); ++j) {
            leftY[j] = y[leftIndices[j]];
        }
        for (int j = 0; j < rightIndices.size(); ++j) {
            rightY[j] = y[rightIndices[j]];
        }
        std::vector<T> leftPredicted = predictSubset(X, leftIndices, i, splitValue, true);
        std::vector<T> rightPredicted = predictSubset(X, rightIndices, i, splitValue, false);
        T error = calculateError(leftPredicted, leftY) + calculateError(rightPredicted, rightY);

        // update best split
        if (error < bestError) {
            bestError = error;
            bestFeatureIndex = i;
            bestSplitValue = splitValue;
        }
    }

    // split using best feature and value
    std::vector<int> leftIndices;
    std::vector<int> rightIndices;
    for (int i = 0; i < numSamples; ++i) {
        if (X[sampleIndices[i]][bestFeatureIndex] <= bestSplitValue) {
            leftIndices.push_back(sampleIndices[i]);
        }
        else {
            rightIndices.push_back(sampleIndices[i]);
        }
    }
    TreeNode* node = new TreeNode();
    node->splitValue = bestSplitValue;
    node->featureIndex = bestFeatureIndex;
    node->leftChild = buildTree(X, y, leftIndices, depth + 1);
    node->rightChild = buildTree(X, y, rightIndices, depth + 1);
    return node;
}

template <typename T>
void GBM<T>::traverseTree(const TreeNode* node, const std::vector<T>& sample, T& prediction) const {
    if (node->leftChild == nullptr && node->rightChild == nullptr) {
        prediction = node->splitValue;
    }
    else {
        int featureIndex = node->featureIndex;
        T splitValue = node->splitValue;
        if (sample[featureIndex] <= splitValue) {
            traverseTree(node->leftChild, sample, prediction);
        }
        else {
            traverseTree(node->rightChild, sample, prediction);
        }
    }
}

template <typename T>
T GBM<T>::calculateSplitValue(const std::vector<std::vector<T>>& X, const std::vector<T>& y,
    const std::vector<int>& sampleIndices, int featureIndex) const {

    int numSamples = sampleIndices.size();
    std::vector<std::pair<T, T>> featureValues(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        featureValues[i].first = X[sampleIndices[i]][featureIndex];
        featureValues[i].second = y[sampleIndices[i]];
    }
    std::sort(featureValues.begin(), featureValues.end());

    // find split that minimizes squared error
    int leftCount = 0;
    int rightCount = numSamples;
    T leftSum = 0.0;
    T rightSum = std::accumulate(y.begin(), y.end(), 0.0);
    T leftSquaredSum = 0.0;
    T rightSquaredSum = std::accumulate(y.begin(), y.end(), [](T x, T y) { return x + y * y; });
    T bestSplitValue = featureValues[0].first;
    T bestError = std::numeric_limits<T>::infinity();
    for (int i = 0; i < numSamples - 1; ++i) {
        T value = featureValues[i].first;
        T label = featureValues[i].second;
        leftCount++;
        rightCount--;
        leftSum += label;
        rightSum -= label;
        leftSquaredSum += label * label;
        rightSquaredSum -= label * label;
        if (value == featureValues[i + 1].first) {
            continue;
        }
        T leftMean = leftSum / leftCount;
        T rightMean = rightSum / rightCount;
        T leftVariance = (leftSquaredSum / leftCount) - (leftMean * leftMean);
        T rightVariance = (rightSquaredSum / rightCount) - (rightMean * rightMean);
        T error = leftVariance * leftCount + rightVariance * rightCount;
        if (error < bestError) {
            bestError = error;
            bestSplitValue = (value + featureValues[i + 1].first) / 2.0;
        }
    }
    return bestSplitValue;
}

template <typename T>
std::vector<T> GBM<T>::predictSubset(const std::vector<std::vector<T>>& X, const std::vector<int>& sampleIndices,
    int featureIndex, T splitValue, bool leftBranch) const {
    std::vector<T> predictions(sampleIndices.size());
    for (int i = 0; i < sampleIndices.size(); ++i) {
        int sampleIndex = sampleIndices[i];
        T prediction;
        if ((leftBranch && X[sampleIndex][featureIndex] <= splitValue) || (!leftBranch && X[sampleIndex][featureIndex] > splitValue)) {
            traverseTree(leftTrees[featureIndex], X[sampleIndex], prediction);
        }
        else {
            traverseTree(rightTrees[featureIndex], X[sampleIndex], prediction);
        }
        predictions[i] = prediction;
    }
    return predictions;
}

template <typename T>
T GBM<T>::calculateError(const std::vector<T>& predictions, const std::vector<T>& labels) const {
    T error = 0.0;
    for (int i = 0; i < predictions.size(); ++i) {
        T residual = labels[i] - predictions[i];
        error += residual * residual;
    }
    return error;
}