//+------------------------------------------------------------------+
//|      probability density functions (pdf)                         |
//+------------------------------------------------------------------+

/* A probability density function (PDF) is a statistical function that gives the probability
of a random variable taking on any particular value.
It is the derivative of the cumulative distribution function and is used to analyze
the probability of a random variable taking on a particular value. */

// author: Christian Suer

#pragma once
#include <cmath>

constexpr double SCALE_FACTOR = 0.707106781;

template<typename T>
class PdfObject{
    public: 
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
                      "T must be either type <float> or <double>");       
        // methods
        static T gaussian(T x_val,T mu=0, T sigma=1);
        static T cauchy(T x_val,T x_peak, T gamma);
        static T laplace(T x_val, T mu=0, T sigma=1);
        static T pareto(T x_val, T alpha=1, T tail_index=1);
        static T lomax(T x_val, T alpha=1, T tail_index=1);
        static T F_distribution(T x_val, T d1, T d2);
        static T poisson(T k, T lambda);
        // constructor
        PdfObject(){};
        // destructor
        ~PdfObject(){};
};

// ------------------------------------------------------------------
// ALIAS CLASS
template<typename T>
class pdf:public PdfObject<T>{};
// ------------------------------------------------------------------


//+------------------------------------------------------------------+
//|                          DEFINITIONS                             |
//+------------------------------------------------------------------+


// gaussian normal probability density function
template<typename T>
T PdfObject<T>::gaussian(T x_val, T mu, T sigma) {
    return (1 / sqrt(2 * M_PI * pow(sigma, 2))) * exp(-0.5 * pow((x_val - mu) / sigma, 2));
}

// Cauchy probability density function
template<typename T>
T PdfObject<T>::cauchy(T x_val, T x_peak, T gamma) {
    return 1 / (M_PI * gamma * (1 + pow((x_val - x_peak) / gamma, 2)));
}

// Laplace probability density function;
// scale factor default: sigma/sqrt(2)=0.707106781
template<typename T>
T PdfObject<T>::laplace(T x_val, T mu, T sigma) {
    static double scale_factor;
    scale_factor = sigma / sqrt(2);
    return exp(-fabs(x_val - mu) / scale_factor) / (2 * scale_factor);
}

// Pareto probability density function;
// note: not defined for x<=0 !!
// note: the tail index can be understood as the scale parameter (default=1)
//       the alpha can be understood as the shape parameter (default=1)
template<typename T>
T PdfObject<T>::pareto(T x_val, T alpha, T tail_index) {
    if (x_val >= tail_index) {
        return (alpha * pow(tail_index, alpha)) / pow(x_val, alpha + 1);
    }
    else { return 0; }
}

// Lomax (=Pareto type II) probability density function;
// note: not defined for x<=0 !!
// note: the tail index can be understood as the scale parameter (default=1)
//       the alpha can be understood as the shape parameter (default=1)
template<typename T>
T PdfObject<T>::lomax(T x_val, T alpha, T tail_index) {
    return (alpha / tail_index) * pow(1 + std::fmax(x_val, 0) / tail_index, -(alpha + 1));
}

template<typename T>
T PdfObject<T>::F_distribution(T x_val, T d1, T d2) {
    if (x_val < 0) {
        return 0;
    }
    T numerator = pow(d1 * x_val, d1 / 2) * pow(d2, d2 / 2);
    T denominator = pow((d1 * x_val + d2), (d1 + d2) / 2) * gamma(d1 / 2) * gamma(d2 / 2);
    return numerator / denominator;
}

template<typename T>
T PdfObject<T>::poisson(T k, T lambda) {
    return pow(lambda, k) * exp(-lambda) / tgamma(k + 1);
}



template<typename T>
T gamma_helper(T x) {
    // Constants
    static const T P[] = { -1.716185138865495, 24.76565080557592,
        -379.80425647094563, 629.3311553128184, 866.9662027904133,
        -31451.272968848367, -36144.413418691172, 66456.14382024054 };
    static const int N = sizeof(P) / sizeof(T);
    static const T SQRT_TWO_PI = sqrt(2 * M_PI);

    T d = 0.0;
    T s = P[N - 1];
    int i;
    for (i = N - 2; i >= 0; i--) {
        d = 1.0 / (x + i + 1 - 0.5 * (N - 1)) + d;
        s += P[i] * d;
    }

    T res = sqrt(2 * M_PI) * pow(x + N - 1.5, x + 0.5) * exp(-x - N + 1.5) * s;
    return res;
}

// gamma function based on the Lanczos approximation
template<typename T>
T gamma(T x) {
    if (x < 0) {
        return std::numeric_limits<T>::quiet_NaN();
    }
    if (x == 0) {
        return std::numeric_limits<T>::infinity();
    }
    if (x < 0.5) {
        return M_PI / (sin(M_PI * x) * gamma_helper(1 - x));
    }
    else {
        return gamma_helper(x - 1);
    }
}