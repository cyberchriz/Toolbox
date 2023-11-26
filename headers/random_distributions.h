//+------------------------------------------------------------------+
//|      random values from a given distribution                     |
//+------------------------------------------------------------------+

// author: 'cyberchriz' (Christian Suer)

#pragma once
#include <cmath>
#include <cstdlib>
#include <chrono>

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


// include .cpp resource (required due to this being a template class)
// -> thus mitigating 'undefined reference' compiler errors
#include "../sources/random_distributions.cpp"