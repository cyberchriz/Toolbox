//+------------------------------------------------------------------+
//|      random values from a given distribution                     |
//+------------------------------------------------------------------+

// author: 'cyberchriz' (Christian Suer)

#ifndef RANDOM_DISTRIBUTIONS_H
#define RANDOM_DISTRIBUTIONS_H
#pragma once

#include <chrono>
#include <cmath>
#include <cstdlib>


namespace rnd {

	constexpr double PI = 3.1415926535897932384626433;

    template<typename T = float> static T gaussian(T mu = 0, T sigma = 1);
    template<typename T = float> static T cauchy(T x_peak = 0, T gamma = 1);
    template<typename T = float> static T uniform(T min = 0, T max = 1);
    template<typename T = float> static T laplace(T mu = 0, T sigma = 1);
    template<typename T = float> static T pareto(T alpha = 1, T tail_index = 1);
    template<typename T = float> static T lomax(T alpha = 1, T tail_index = 1);
    template<typename T = float> static T binary();
    template<typename T = float> static T sign();
    template<typename T = float> static uint32_t seed32();


    //+------------------------------------------------------------------+
    //|                          DEFINITIONS                             |
    //+------------------------------------------------------------------+


    // get random value from a gaussian normal distribution with a given µ and sigma
    template<typename T>
    T gaussian(T mu, T sigma) {
        static double M_PI = 3.141592653589793;
        double random = (double)rand() / RAND_MAX;                      // get random value within range 0-1
        random /= sqrt(2 * M_PI * pow(sigma, 2));                       // reduce to the top of the distribution (f(x_val=mu))
        char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;                 // get random algebraic sign
        return T((mu + sigma * sqrt(-2 * log(random / (1 / sqrt(2 * M_PI * pow(sigma, 2)))))) * sign);
    }

    // get random value from a Cauchy distribution with a given x_peak and gamma
    template<typename T>
    T cauchy(T x_peak, T gamma) {
        double random = (double)rand() / RAND_MAX;                      // random value within range 0-1
        random /= PI * gamma;                                         // reduce to the top of the distribution (=f(x_val=x_peak))
        char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;                 // get random algebraic sign   
        return T((sqrt(gamma / (random * PI) - pow(gamma, 2)) + x_peak) * sign);
    }

    // get random value from a uniform distribution
    template<typename T>
    T uniform(T min, T max) {
        double random = (double)rand() / RAND_MAX;                      // random value within range +/- 1
        random *= (max - min);                                          // expand range
        return T(random + min);                                         // shift by lower margin
    }

    // get random value from a Laplace distribution
    template<typename T>
    T laplace(T mu, T sigma) {
        double scale_factor = sigma / sqrt(2);
        double random = (double)rand() / RAND_MAX;                      // random value within range 0-1
        random /= 2 * scale_factor;                                     // reduce to top of distribution (f(x_val=mu)
        char sign = rand() > (0.5 * RAND_MAX) ? 1 : -1;                 // get random algebraic sign
        return T(mu + scale_factor * log(random * 2 * scale_factor) * sign);
    }

    // get random value from a Pareto distribution
    template<typename T>
    T pareto(T alpha, T tail_index) {
        double random = (double)rand() / RAND_MAX;                      // random value within range 0-1
        random *= (alpha * pow(tail_index, alpha)) / pow(tail_index, alpha + 1); // top of distribution is given for x_val=tail_index
        return T(pow((alpha * pow(tail_index, alpha)) / random, 1 / (alpha + 1)));
    }

    // get random value from a Lomax distribution
    template<typename T>
    T lomax(T alpha, T tail_index) {
        double random = (double)rand() / RAND_MAX;                      // random value within range 0-1
        random *= (alpha / tail_index) * pow(1 / tail_index, -(alpha + 1));
        return T(tail_index * (pow((random * tail_index) / alpha, -1 / (alpha + 1)) - 1));
    }

    // random binary
    template<typename T>
    T binary() {
        return T((int)rand() % 2);
    }

    // random sign
    template<typename T>
    T sign() {
        return T(rand() > (0.5 * RAND_MAX) ? 1 : -1);                   // get random algebraic sign
    }

    // Function to extract a 32-bit seed from std::chrono::time_point
    template<typename T>
    uint32_t seed32() {
        std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return static_cast<uint32_t>(ms_since_epoch.count());
    }
}
#endif