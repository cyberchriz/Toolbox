//+------------------------------------------------------------------+
//|          cumulative distribution functions (cdf)                 |
//+------------------------------------------------------------------+

/* A cumulative distribution function gives the probability of a random variable
taking on values less than or equal to a given value. */

// author: Christian Suer (cyberchriz)

#ifndef CUMULATIVE_DISTRIBUTION_FUNCTIONS_H
#define CUMULATIVE_DISTRIBUTION_FUNCTIONS_H

#include <cmath>


template<typename T>
class CdfObject{
    public:
        static_assert(std::is_same<T, float>::value ||
                      std::is_same<T, double>::value,
                      "T must be either type <float> or <double>");    
        // methods
        static T gaussian(T x_val, T mu=0, T sigma=1);
        static T cauchy(T x_val, T x_peak=0, T gamma=1);
        static T laplace(T x_val,T mu=0, T sigma=1);
        static T pareto(T x_val, T alpha=1, T tail_index=1);
        static T lomax(T x_val,T alpha=1, T tail_index=1);
        static T F_distribution(T x_val, T d1, T d2);        
        // constructor
        CdfObject<T>(){};
        // destructor
        ~CdfObject<T>(){};
};

// ------------------------------------------------------------------
// ALIAS CLASS
template<typename T>
class cdf:public CdfObject<T>{};
// ------------------------------------------------------------------

//+------------------------------------------------------------------+
//|                          DEFINITIONS                             |
//+------------------------------------------------------------------+


// returns the probability of a random value
// as part of a gaussian normal distribution  (with a given µ and sigma)
// being less or equal than the given 'x_val'
template<typename T>
T CdfObject<T>::gaussian(T x_val, T mu, T sigma) {
    return 0.5 * (1 + erf((x_val - mu) / (sigma * sqrt(2))));
}

// returns the probability of a random value
// as part of a Cauchy distribution  (with given parameter x_peak and sigma)
// being less or equal than the given 'x_val'
template<typename T>
T CdfObject<T>::cauchy(T x_val, T x_peak, T gamma) {
    return 0.5 + atan((x_val - x_peak) / gamma) / M_PI;
}

// returns the probability of a random value
// as part of a Laplace distribution  (with given µ)
// being less or equal than the given 'x_val';
// the scale_factor is sigma/sqrt(2)=0.707106781 by default
template<typename T>
T CdfObject<T>::laplace(T x_val, T mu, T sigma) {
    static double scale_factor= 0.707106781;
    scale_factor = sigma / sqrt(2);
    if (x_val < mu)
    {
        return 0.5 * exp((x_val - mu) / scale_factor);
    }
    else
    {
        return 1 - 0.5 * exp(-(x_val - mu) / scale_factor);
    }
}

// returns the probability of a random value
// as part of a Pareto distribution
// being less or equal than the given 'x_val';
// alpha is 1 by default;
// the tail_index is 1 by default;
// note: not defined for x<=0!!
template<typename T>
T CdfObject<T>::pareto(T x_val, T alpha, T tail_index) {
    if (x_val >= tail_index)
    {
        return 1 - pow(tail_index / x_val, alpha);
    }
    else
    {
        return 0;
    }
}

// returns the probability of a random value
// as part of a Lomax distribution
// being less or equal than the given 'x_val';
// alpha is 1 by default;
// the tail_index is 1 by default;
// note: not defined for x<=0!!
template<typename T>
T CdfObject<T>::lomax(T x_val, T alpha, T tail_index) {
    return 1 - pow(1 + std::fmax(0, x_val) / tail_index, -alpha);
}


template<typename T>
T CdfObject<T>::F_distribution(T x_val, T d1, T d2) {
    if (x_val < 0) {
        return 0;
    }
    return regularized_beta(d1 / 2, d2 / 2, d1 * x_val / (d1 * x_val + d2));
}



// Helper function for the regularized incomplete beta function
template <typename T>
T beta_inc(T a, T b, T x) {
    const int MAXIT = 100;
    const T EPS = std::numeric_limits<T>::epsilon();
    const T FPMIN = std::numeric_limits<T>::min() / EPS;
    const T fpmin = static_cast<T>(FPMIN);
    const T xk = 1 - x;
    T ap = a, bp = b, am = 1, bm = 1, az = 1, qab = a + b, qap = a + 1, qam = a - 1, bz = 1 - qab * x / qap;
    T aold, em, tem, d = 0;

    for (int m = 1; m <= MAXIT; ++m) {
        em = static_cast<T>(m);
        tem = em + em;
        d = em * (b - em) * x / ((ap + tem) * (a + tem));
        const T apb = ap + bp;
        const T app = ap + em;
        const T apm = ap - em;
        const T bpp = bp + em;
        const T bpm = bp - em;
        az = bz + d * az;
        const T bpz = bz + d / bm * az;
        am = az / ap;
        bm = bz / bp;
        const T aaz = -(ap + em) * (qam + em) * x / ((a + tem) * (qap + em));
        const T bbz = bpz + d * bpz / bm;
        const T abz = am * bbz + bm * aaz;
        const T aold = az;
        az = bbz / abz;
        bz = bpz / abz;
        if (std::abs(az - aold) < EPS * std::abs(az)) {
            return az;
        }
    }
    throw std::runtime_error("beta_inc failed to converge");
}

// Regularized incomplete beta function
template <typename T>
T regularized_beta(T a, T b, T x) {
    if (x < 0.0 || x > 1.0) {
        throw std::invalid_argument("x must be in [0, 1]");
    }
    if (x == 0.0) {
        return static_cast<T>(0.0);
    }
    if (x == 1.0) {
        return static_cast<T>(1.0);
    }
    const T bt = std::exp(std::lgamma(a + b) - std::lgamma(a) - std::lgamma(b) + a * std::log(x) + b * std::log(1 - x));
    if (x < (a + 1) / (a + b + 2)) {
        return bt * beta_inc(a, b, x) / a;
    }
    else {
        return 1 - bt * beta_inc(b, a, 1 - x) / b;
    }
}

#endif