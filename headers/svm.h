// Support Vector Machines

#ifndef SVM_H
#define SVM_H

#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdlib>

template <typename T>
class SVM {
public:
    explicit SVM(int numFeatures, T regularizationParameter, T tolerance, int maxIterations);
    void train(const std::vector<std::vector<T>>& data, const std::vector<T>& labels);
    T predict(const std::vector<T>& dataPoint) const;

private:
    T kernelFunction(const std::vector<T>& dataPoint1, const std::vector<T>& dataPoint2) const;

    int numFeatures;
    T regularizationParameter;
    T tolerance;
    int maxIterations;
    std::vector<T> alphas;
    std::vector<std::vector<T>> trainingData;
    T bias;
};

//-------------------------------------------------------------------
//                        CONSTRUCTOR                    
//-------------------------------------------------------------------

template <typename T>
SVM<T>::SVM(int numFeatures, T regularizationParameter, T tolerance, int maxIterations) :
    numFeatures(numFeatures),
    regularizationParameter(regularizationParameter),
    tolerance(tolerance),
    maxIterations(maxIterations),
    alphas(numFeatures, 0),
    bias(0) {}

//-------------------------------------------------------------------
//                        TRAIN METHOD                    
//-------------------------------------------------------------------

template <typename T>
void SVM<T>::train(const std::vector<std::vector<T>>& data, const std::vector<T>& labels) {
    int numDataPoints = data.size();
    trainingData = data;

    // Create kernel matrix
    std::vector<std::vector<T>> kernelMatrix(numDataPoints, std::vector<T>(numDataPoints));
    for (int i = 0; i < numDataPoints; i++) {
        for (int j = 0; j < numDataPoints; j++) {
            kernelMatrix[i][j] = kernelFunction(data[i], data[j]);
        }
    }

    // Initialize alpha vector
    alphas.assign(numDataPoints, 0);

    // Sequential Minimal Optimization (SMO) algorithm
    int iteration = 0;
    while (iteration < maxIterations) {
        bool alphaChanged = false;

        for (int i = 0; i < numDataPoints; i++) {
            T error = bias;

            for (int j = 0; j < numDataPoints; j++) {
                error += alphas[j] * labels[j] * kernelMatrix[i][j];
            }

            error -= labels[i];

            if ((labels[i] * error < -tolerance && alphas[i] < regularizationParameter) ||
                (labels[i] * error > tolerance && alphas[i] > 0)) {
                int j = i;

                while (j == i) {
                    j = rand() % numDataPoints;
                }

                T error2 = bias;

                for (int k = 0; k < numDataPoints; k++) {
                    error2 += alphas[k] * labels[k] * kernelMatrix[j][k];
                }

                error2 -= labels[j];

                T oldAlphaI = alphas[i];
                T oldAlphaJ = alphas[j];
                T L, H;

                if (labels[i] != labels[j]) {
                    L = std::max((T)0, alphas[j] - alphas[i]);
                    H = std::min(regularizationParameter, regularizationParameter + alphas[j] - alphas[i]);
                }
                else {
                    L = std::max((T)0, alphas[i] + alphas[j] - regularizationParameter);
                    H = std::min(regularizationParameter, alphas[i] + alphas[j]);
                }

                if (L == H) continue;

                T eta = 2 * kernelMatrix[i][j] - kernelMatrix[i][i] - kernelMatrix[j][j];
                if (eta >= 0) continue;

                alphas[j] -= labels[j] * (error - error2) / eta;
                alphas[j] = std::min(
                    alphas[j] = std::min(std::max(alphas[j], L), H);

                if (std::abs(alphas[j] - oldAlphaJ) < tolerance) continue;

                alphas[i] += labels[i] * labels[j] * (oldAlphaJ - alphas[j]);

                T b1 = bias - error - labels[i] * (alphas[i] - oldAlphaI) * kernelMatrix[i][i] - labels[j] * (alphas[j] - oldAlphaJ) * kernelMatrix[i][j];
                T b2 = bias - error2 - labels[i] * (alphas[i] - oldAlphaI) * kernelMatrix[i][j] - labels[j] * (alphas[j] - oldAlphaJ) * kernelMatrix[j][j];

                if (0 < alphas[i] && alphas[i] < regularizationParameter) {
                    bias = b1;
                }
                else if (0 < alphas[j] && alphas[j] < regularizationParameter) {
                    bias = b2;
                }
                else {
                    bias = (b1 + b2) / 2;
                }

                alphaChanged = true;
            }
        }

        if (!alphaChanged) {
            break;
        }

        iteration++;
    }
}

//-------------------------------------------------------------------
//                        PREDICT METHOD                  
//-------------------------------------------------------------------

template <typename T>
T SVM<T>::predict(const std::vector<T>& dataPoint) const {
    T sum = 0;

    for (int i = 0; i < alphas.size(); i++) {
        sum += alphas[i] * kernelFunction(dataPoint, trainingData[i]);
    }

    sum -= bias;

    return sum >= 0 ? 1 : -1;
}

//-------------------------------------------------------------------
//|                      KERNEL FUNCTION                  
//-------------------------------------------------------------------

template <typename T>
T SVM<T>::kernelFunction(const std::vector<T>& dataPoint1, const std::vector<T>& dataPoint2) const {
    T dotProduct = 0;

    for (int i = 0; i < dataPoint1.size(); i++) {
        dotProduct += dataPoint1[i] * dataPoint2[i];
    }

    return dotProduct;
}

#endif