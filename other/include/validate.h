// functions for NaN/Inf error mitigation

// author: 'cyberchriz' (Christian Suer)


// preprocessor directives
#ifndef VALIDATE_H
#define VALIDATE_H
#ragma once

#include <cmath>


// =============================================
// replace expression with given alternative
// =============================================

template<typename T>
T validate(T expression, T alternative){
    if (std::isnan(expression) || std::isinf(expression)){
        return alternative;
    }
    else{
        return expression;
    }
}

// =============================================
// replace expression with automatic alternative
// =============================================

double validate(double expression){
    if (expression!=expression){return DBL_MIN;}
    if (std::isinf(expression)){
        if (expression>DBL_MAX){
            return DBL_MAX;
        }
        else {
            return -DBL_MAX;
        }
    }
}

long double validate(long double expression){
    if (expression!=expression){return LDBL_MIN;}
    if (std::isinf(expression)){
        if (expression>LDBL_MAX){
            return LDBL_MAX;
        }
        else {
            return -LDBL_MAX;
        }
    }
}

float validate(float expression){
    if (expression!=expression){return FLT_MIN;}
    if (std::isinf(expression)){
        if (expression>FLT_MAX){
            return FLT_MAX;
        }
        else {
            return -FLT_MAX;
        }
    }
}

int validate(int expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){
        if (expression>UINT32_MAX){
            return UINT32_MAX;
        }
        else {
            return -UINT32_MAX;
        }
    }
}

uint validate(uint expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){return UINT32_MAX;}
}

long validate(long expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){
        if (expression>__LONG_MAX__){
            return __LONG_MAX__;
        }
        else {
            return -__LONG_MAX__;
        }
    }
}

u_char validate(u_char expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){return 255;}
}

char validate(char expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){
        if (expression>127){
            return 127;
        }
        else {
            return -127;
        }
    }
}

short validate(short expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){
        if (expression>SHRT_MAX){
            return SHRT_MAX;
        }
        else {
            return -SHRT_MAX;
        }
    }
}

ushort validate(ushort expression){
    if (expression!=expression){return 0;}
    if (std::isinf(expression)){return 2*SHRT_MAX;}
}

// =============================================
// pass by reference, with automatic alternative
// =============================================

void validate_r(double& expression){
    if (expression!=expression){expression = DBL_MIN;}
    if (std::isinf(expression)){
        if (expression>DBL_MAX){
            expression = DBL_MAX;
        }
        else {
            expression = -DBL_MAX;
        }
    }
}

void validate_r(long double& expression){
    if (expression!=expression){expression= LDBL_MIN;}
    if (std::isinf(expression)){
        if (expression>LDBL_MAX){
            expression = LDBL_MAX;
        }
        else {
            expression = -LDBL_MAX;
        }
    }
}

void validate_r(float& expression){
    if (expression!=expression){expression = FLT_MIN;}
    if (std::isinf(expression)){
        if (expression>FLT_MAX){
            expression = FLT_MAX;
        }
        else {
            expression = -FLT_MAX;
        }
    }
}

void validate_r(int& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){
        if (expression>UINT32_MAX){
            expression = UINT32_MAX;
        }
        else {
            expression = -UINT32_MAX;
        }
    }
}

void validate_r(uint& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){expression = UINT32_MAX;}
}

void validate_r(long& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){
        if (expression>__LONG_MAX__){
            expression = __LONG_MAX__;
        }
        else {
            expression = -__LONG_MAX__;
        }
    }
}

void validate_r(u_char& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){expression = 255;}
}

void validate_r(char& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){
        if (expression>127){
            expression = 127;
        }
        else {
            expression = -127;
        }
    }
}

void validate_r(short& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){
        if (expression>SHRT_MAX){
            expression = SHRT_MAX;
        }
        else {
            expression = -SHRT_MAX;
        }
    }
}

void validate_r(ushort& expression){
    if (expression!=expression){expression = 0;}
    if (std::isinf(expression)){expression = 2*SHRT_MAX;}
}

#endif