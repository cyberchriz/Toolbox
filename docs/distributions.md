[[return to main page]](../../../README.md)
## Distributions
Usage:

-`#include <distributions.h>`
- or include any of the sub-libraries
    - `#include <random_distributions.h>`
    - `#include <cumulative_distribution_functions.h>`
    - `#include <probability_density_functions.h>`
- or include as part of `<datascience.h>`

Make sure to update the include path or use relative paths instead.

### Random Numbers
Class Name: `Random`, alias `rnd`

Example:
```
int myNum1 = rnd<int>::uniform(0,100);
double myNum2 = rnd<double>::gaussian();
```
Public Methods:
```
static T gaussian(T mu=0, T sigma=1);
static T cauchy(T x_peak=0, T gamma=1);
static T uniform(T min=0, T max=1);
static T laplace(T mu=0, T sigma=1);
static T pareto(T alpha=1, T tail_index=1);
static T lomax(T alpha=1, T tail_index=1);
static T binary();     
static T sign();
```

### Cumulative Distribution Functions
Description: A cumulative distribution function gives the probability of a random variable (as element of a specified distribution) taking on values less than or equal to a given value.

Class Name: `CdfObject`, alias `cdf`

Public Methods:
```
static T gaussian(T x_val, T mu=0, T sigma=1);
static T cauchy(T x_val, T x_peak=0, T gamma=1);
static T laplace(T x_val,T mu=0, T sigma=1);
static T pareto(T x_val, T alpha=1, T tail_index=1);
static T lomax(T x_val,T alpha=1, T tail_index=1);
```


### Probability Density Functions
Description: A probability density function (PDF) is a statistical function that gives the probability
of a random variable (as element of a specified distribution) taking on any particular value.
It is the derivative of the cumulative distribution function.

Class Name: `PdfObject`, alias `pdf`

Public Methods:
```
static T gaussian(T x_val,T mu=0, T sigma=1);
static T cauchy(T x_val,T x_peak, T gamma);
static T laplace(T x_val, T mu=0, T sigma=1);
static T pareto(T x_val, T alpha=1, T tail_index=1);
static T lomax(T x_val, T alpha=1, T tail_index=1);
```

[[return to main page]](../../../README.md)