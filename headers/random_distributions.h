//+------------------------------------------------------------------+
//|      random values from a given distribution                     |
//+------------------------------------------------------------------+

// author: 'cyberchriz' (Christian Suer)

#ifndef RANDOM_DISTRIBUTIONS_H
#define RANDOM_DISTRIBUTIONS_H

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <math.h>

// singleton class(!)
template<typename T>
class Random {
    private:
        // private constructor
        Random(){
            auto t = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::srand(t);
        };
    public:   
        static T gaussian(T mu=0, T sigma=1);
        static T cauchy(T x_peak=0, T gamma=1);
        static T uniform(T min=0, T max=1);
        static T laplace(T mu=0, T sigma=1);
        static T pareto(T alpha=1, T tail_index=1);
        static T lomax(T alpha=1, T tail_index=1);
        static T binary();     
        static T sign();
        static Random& getInstance(){
            static Random instance;
            return instance;
        }
        // delete copy constructor
        Random(const Random&) = delete;
        // delete implementaion of assignment operator
        Random& operator=(const Random&) = delete;
};

// ALIAS CLASS
template<typename T> class rnd: public Random<T>{};

// example of instantiation:
// Random<double>& random = Random<double>::getInstance();




//+------------------------------------------------------------------+
//|                          DEFINITIONS                             |
//+------------------------------------------------------------------+


// get random value from a gaussian normal distribution with a given µ and sigma
template<typename T>
T Random<T>::gaussian(T mu, T sigma) {
    static double M_PI = 3.141592653589793;
    double random = (double)rand() / RAND_MAX;                // get random value within range 0-1
    random /= sqrt(2 * M_PI * pow(sigma, 2));                      // reduce to the top of the distribution (f(x_val=mu))
    char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;               // get random algebraic sign
    return T((mu + sigma * sqrt(-2 * log(random / (1 / sqrt(2 * M_PI * pow(sigma, 2)))))) * sign);
}

// get random value from a Cauchy distribution with a given x_peak and gamma
template<typename T>
T Random<T>::cauchy(T x_peak, T gamma) {
    double random = (double)rand() / RAND_MAX;                // random value within range 0-1
    random /= M_PI * gamma;                                     // reduce to the top of the distribution (=f(x_val=x_peak))
    char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;               // get random algebraic sign   
    return T((sqrt(gamma / (random * M_PI) - pow(gamma, 2)) + x_peak) * sign);
}

// get random value from a uniform distribution
template<typename T>
T Random<T>::uniform(T min, T max) {
    double random = (double)rand() / RAND_MAX;                // random value within range +/- 1
    random *= (max - min);                                      // expand range
    return T(random + min);                                   // shift by lower margin
}

// get random value from a Laplace distribution
template<typename T>
T Random<T>::laplace(T mu, T sigma) {
    double scale_factor = sigma / sqrt(2);
    double random = (double)rand() / RAND_MAX;                // random value within range 0-1
    random /= 2 * scale_factor;                                 // reduce to top of distribution (f(x_val=mu)
    char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;               // get random algebraic sign
    return T(mu + scale_factor * log(random * 2 * scale_factor) * sign);
}

// get random value from a Pareto distribution
template<typename T>
T Random<T>::pareto(T alpha, T tail_index) {
    double random = (double)rand() / RAND_MAX;                // random value within range 0-1
    random *= (alpha * pow(tail_index, alpha)) / pow(tail_index, alpha + 1); // top of distribution is given for x_val=tail_index
    return T(pow((alpha * pow(tail_index, alpha)) / random, 1 / (alpha + 1)));
}

// get random value from a Lomax distribution
template<typename T>
T Random<T>::lomax(T alpha, T tail_index) {
    double random = (double)rand() / RAND_MAX;                // random value within range 0-1
    random *= (alpha / tail_index) * pow(1 / tail_index, -(alpha + 1));
    return T(tail_index * (pow((random * tail_index) / alpha, -1 / (alpha + 1)) - 1));
}

// random binary
template<typename T>
T Random<T>::binary() {
    return T((int)rand() % 2);
}

// random sign
template<typename T>
T Random<T>::sign() {
    return T(rand() > (0.5 * RAND_MAX) ? 1 : -1);               // get random algebraic sign
}

#endif