#include "../headers/activation_functions.h"
constexpr auto M_E = 2.71828182845904523536;

// +------------------------------------------------------------------+
// |       function ELU / ELU_drv                                     |
// +------------------------------------------------------------------+
double Activation::ELU(double z){
    static double alpha = 0.01;
    return z>0 ? z : alpha*(exp(z)-1);
}   

double Activation::ELU_drv(double z){
    static double alpha=0.01;
    return z>0 ? 1 : alpha*exp(z);
}   

// +------------------------------------------------------------------+
// |       function sigmoid / sigmoid_drv                             |
// +------------------------------------------------------------------+   
double Activation::sigmoid(double z){
    return 1/(1+exp(-z));
}

double Activation::sigmoid_drv(double z){
   return exp(z)/pow(exp(z)+1,2);
}   

// +------------------------------------------------------------------+
// |       function oblique sigmoid / obl. sigmoid_drv (custom)       |
// +------------------------------------------------------------------+   
double Activation::oblique_sigmoid(double z){
    static double alpha=0.01;
    return 1/(1+exp(-z))+alpha*z;
}

double Activation::oblique_sigmoid_drv(double z){
   static double alpha=0.01;
   return exp(z)/(pow(exp(z)+1,2)) + alpha;
} 

// +------------------------------------------------------------------+
// |       function ReLU / ReLU_drv                                   |
// +------------------------------------------------------------------+      
double Activation::ReLU(double z){
    return z>0 ? z : 0;
}
      
double Activation::ReLU_drv(double z){
    return z>0;
}

// +------------------------------------------------------------------+
// |       function LReLU / LReLU_drv                                 |
// +------------------------------------------------------------------+
double Activation::LReLU(double z){
    static double alpha=0.01;
    return z>0 ? z : alpha*z;
}
      
double Activation::LReLU_drv(double z){
    static double alpha = 0.01;
    return z>0 ? 1 : alpha;
}
    
// +------------------------------------------------------------------+
// |       function modtanh / modtanh_drv                             |
// +------------------------------------------------------------------+
double Activation::modtanh(double z){
    return tanh(z);
}

double Activation::modtanh_drv(double z){
    return 1-pow(tanh(z),2);
}

// +------------------------------------------------------------------+
// |       function oblique_tanh / oblique_tanh_drv (custom)          |
// +------------------------------------------------------------------+
double Activation::oblique_tanh(double z){
    static double alpha=0.01;  
    return tanh(z)+alpha*z; 
}
     
double Activation::oblique_tanh_drv(double z){
    static double alpha=0.01;
    return 1-pow(tanh(z),2)+alpha;
}
  
// +------------------------------------------------------------------+
// |       function tanh rectifier / _drv (custom)                    |
// +------------------------------------------------------------------+
double Activation::tanh_rectifier(double z){
    double alpha=0.01;
    return z>0 ? tanh(z)+alpha*z : alpha*tanh(z);
}
     
double Activation::tanh_rectifier_drv(double z){
    double alpha=0.01;
    return z>0 ? 1-pow(tanh(z),2)+alpha : alpha*(1-pow(tanh(z),2))+alpha;
}  
  
//+------------------------------------------------------------------+
//|      function arctan / arctan_drv                                |
//+------------------------------------------------------------------+
double Activation::arctan(double z){
    return atan(z);
}
  
double Activation::arctan_drv(double z){
    return 1/(z*z+1);
}

//+------------------------------------------------------------------+
//|      function arsinh / arsinh_drv                                |
//+------------------------------------------------------------------+
double Activation::arsinh(double z){
    return asinh(z);
}

double Activation::arsinh_drv(double z){
    return 1/sqrt(z*z+1);
}

//+------------------------------------------------------------------+
//|      function softsign / softsign_drv                            |
//+------------------------------------------------------------------+
double Activation::softsign(double z){
    return z/(1+fabs(z));
}
  
double Activation::softsign_drv(double z){
    return 1/pow(1+fabs(z),2);
}

//+------------------------------------------------------------------+
//|      function ISRU / ISRU_drv                                    |
//+------------------------------------------------------------------+
double Activation::ISRU(double z){
    static double alpha=1;
    return z/sqrt(1+alpha*z*z);
}
  
double Activation::ISRU_drv(double z)  {
    static double alpha=1;
    return pow(1/sqrt(1+alpha*pow(z,2)),3);
}

//+------------------------------------------------------------------+
//|      function ISRLU / ISRLU_drv                                  |
//+------------------------------------------------------------------+
// note: ISRLU="inverse square root linear unit"
double Activation::ISRLU(double z){
    static double alpha=1;
    return z<0 ? z/sqrt(1+alpha*pow(z,2)) : z;
}
  
double Activation::ISRLU_drv(double z){
    static double alpha=1;
    return z<0 ? pow(1/sqrt(1+alpha*pow(z,2)),3) : 1;
}

//+------------------------------------------------------------------+
//|      function softplus / softplus_drv                            |
//+------------------------------------------------------------------+
double Activation::softplus(double z){
    return log(1+exp(z));
}
  
double Activation::softplus_drv(double z){
    return 1/(1+exp(-z));
}

//+------------------------------------------------------------------+
//|      function bentident / bentident_drv                          |
//+------------------------------------------------------------------+
double Activation::bentident(double z){
    return (sqrt(z*z+1)-1)/2+z;
}
  
double Activation::bentident_drv(double z){
    return z/(2*sqrt(z*z+1))+1;
}

//+------------------------------------------------------------------+
//|      function sinusoid / sinusoid_drv                            |
//+------------------------------------------------------------------+
double Activation::sinusoid(double z){
    return sin(z);
}
  
double Activation::sinusoid_drv(double z){
    return cos(z);
}

//+------------------------------------------------------------------+
//|      function sinc / sinc_drv                                    |
//+------------------------------------------------------------------+
double Activation::sinc(double z){
    return z==0 ? 1 : sin(z)/z;
}
  
double Activation::sinc_drv(double z){
    return z==0 ? 0 : cos(z)/z-sin(z)/(z*z);
}     

//+------------------------------------------------------------------+
//|      function gaussian / gaussian_drv                            |
//+------------------------------------------------------------------+
double Activation::gaussian(double z){
    return exp(-z*z);
}
  
double Activation::gaussian_drv(double z){
   return -2*z*pow(M_E,-z*z);
}

//+------------------------------------------------------------------+
//|      function differentiable hardstep (custom) / _drv            |
//+------------------------------------------------------------------+
double Activation::diff_hardstep(double z){
    static double alpha=0.01;
    return z>0 ? 1+alpha*z : 0;
}

double Activation::diff_hardstep_drv(double z){
    static double alpha=0.01;
    return z>0 ? alpha : 0;
}
  
//+------------------------------------------------------------------+
//|      function leaky differentiable hardstep (custom) / _drv      |
//+------------------------------------------------------------------+
double Activation::leaky_diff_hardstep(double z){
   static double alpha=0.01;
   return z>=0 ? 1+alpha*z : alpha*z;
}

double Activation::leaky_diff_hardstep_drv(double z){
    static double alpha=0.01;
    return alpha;
}  
  
//+------------------------------------------------------------------+
//|      function log rectifier (custom) / _drv                      |
//+------------------------------------------------------------------+
double Activation::log_rectifier(double z){
    return z>0 ? log(z+1) : 0;
}
  
double Activation::log_rectifier_drv(double z){
    return z>0 ? 1/(z+1) : 0;
}
  
//+------------------------------------------------------------------+
//|      function leaky log rectifier (custom) / _drv                |
//+------------------------------------------------------------------+
double Activation::leaky_log_rectifier(double z){
    static double alpha=0.01;
    return z>0 ? log(z+1) : alpha*z;
}
  
double Activation::leaky_log_rectifier_drv(double z){
    static double alpha=0.01;
    return z>0 ? 1/(z+1) : alpha;
}  
  
//+------------------------------------------------------------------+
//|      function ramp / ramp_drv                                    |
//+------------------------------------------------------------------+
double Activation::ramp(double z){
    if (z>1){return 1;}
    else if (z>-1){return z;}
    else return -1;
}
  
double Activation::ramp_drv(double z){
    if (z>1){return 0;}
    else if (z>-1){return 1;}
    else return 0;
}

//+------------------------------------------------------------------+
//|      return act.function as string variable equivalent           |
//+------------------------------------------------------------------+
std::string Activation::to_string(ACT_FUNC f)
  {
   switch(f)
     {
      case F_IDENT:     return "identity";
      case F_SIGMOID:   return "sigmoid (logistic)";
      case F_OBLIQUE_SIGMOID: return "oblique sigmoid (custom)";
      case F_ELU:       return "exponential linear unit (ELU)";
      case F_RELU:      return "rectified linear unit (ReLU)";
      case F_LRELU:     return "leaky rectified linear unit (ReLU)";
      case F_TANH:      return "hyperbolic tangent (tanh)";
      case F_OBLIQUE_TANH: return "oblique hyperbolic tangent (f(x)=tanh(x)+0.01x)";
      case F_TANH_RECTIFIER: return "tanh rectifier (custom, x<0: 0.01*tanh(x); x>=0: tanh(x)+0.01x)";
      case F_ARCTAN:    return "arcus tangent (arctan)";
      case F_ARSINH:    return "area sinus hyperbolicus (inv. hyperbol.sine)";
      case F_SOFTSIGN:  return "softsign";
      case F_ISRU:      return "inverse square root unit (ISRU)";
      case F_ISRLU:     return "inverse square root linear unit (ISRLU)";
      case F_SOFTPLUS:  return "softplus";
      case F_BENTIDENT: return "bent identity";
      case F_SINUSOID:  return "sinusoid";
      case F_SINC:      return "cardinal sine (sinc)";
      case F_GAUSSIAN:  return "gaussian";
      case F_DIFFERENTIABLE_HARDSTEP: return "differentiable hardstep";
      case F_LEAKY_DIFF_HARDSTEP: return "leaky differentiable hardstep";
      case F_SOFTMAX:   return "normalized exponential (softmax)";
      case F_LOG_RECTIFIER: return "log rectifier (x<=0: 0, x>0: ln(x+1))";
      case F_LEAKY_LOG_RECTIFIER: return "leaky log rectifier (x<=0: 0.01x, x>0: ln(x+1))";
      case F_RAMP:      return "ramp";
      default:          return "";
     }
  }

// +------------------------------------------------------------------+
// |       function Activate / DeActivate                             |
// +------------------------------------------------------------------+
double Activation::function(double x, ACT_FUNC f){
    switch(f){
        case F_IDENT:     return x;
        case F_SIGMOID:   return sigmoid(x);
        case F_ELU:       return ELU(x);
        case F_RELU:      return ReLU(x);
        case F_LRELU:     return LReLU(x);
        case F_TANH:      return modtanh(x);
        case F_OBLIQUE_TANH: return oblique_tanh(x);
        case F_TANH_RECTIFIER: return tanh_rectifier(x);
        case F_ARCTAN:    return arctan(x);
        case F_ARSINH:    return arsinh(x);
        case F_SOFTSIGN:  return softsign(x);
        case F_ISRU:      return ISRU(x);
        case F_ISRLU:     return ISRLU(x);
        case F_SOFTPLUS:  return softplus(x);
        case F_BENTIDENT: return bentident(x);
        case F_SINUSOID:  return sinusoid(x);
        case F_SINC:      return sinc(x);
        case F_GAUSSIAN:  return gaussian(x);
        case F_DIFFERENTIABLE_HARDSTEP: return diff_hardstep(x);
        case F_LEAKY_DIFF_HARDSTEP: return leaky_diff_hardstep(x);
        case F_OBLIQUE_SIGMOID: return oblique_sigmoid(x);
        case F_LOG_RECTIFIER: return log_rectifier(x);
        case F_LEAKY_LOG_RECTIFIER: return leaky_log_rectifier(x);
        case F_RAMP:      return ramp(x);
        default:          return x; //="identity function"
    }
}

double Activation::derivative(double x, ACT_FUNC f){
    switch(f){
        case F_IDENT:     return 1;
        case F_SIGMOID:   return sigmoid_drv(x);
        case F_ELU:       return ELU_drv(x);
        case F_RELU:      return LReLU_drv(x);
        case F_TANH:      return modtanh_drv(x);
        case F_OBLIQUE_TANH: return oblique_tanh_drv(x);
        case F_TANH_RECTIFIER: return tanh_rectifier_drv(x);
        case F_ARCTAN:    return arctan_drv(x);
        case F_ARSINH:    return arsinh_drv(x);
        case F_SOFTSIGN:  return softsign_drv(x);
        case F_ISRU:      return ISRU_drv(x);
        case F_ISRLU:     return ISRLU_drv(x);
        case F_SOFTPLUS:  return softplus_drv(x);
        case F_BENTIDENT: return bentident_drv(x);
        case F_SINUSOID:  return sinusoid_drv(x);
        case F_SINC:      return sinc_drv(x);
        case F_GAUSSIAN:  return gaussian_drv(x);
        case F_DIFFERENTIABLE_HARDSTEP: return diff_hardstep_drv(x);
        case F_LEAKY_DIFF_HARDSTEP: return leaky_diff_hardstep_drv(x);
        case F_OBLIQUE_SIGMOID: return oblique_sigmoid_drv(x);
        case F_LOG_RECTIFIER: return log_rectifier_drv(x);
        case F_LEAKY_LOG_RECTIFIER: return leaky_log_rectifier_drv(x);
        case F_RAMP:      return ramp_drv(x);
        default:          return 1; //="identity function" derivative
    }
}