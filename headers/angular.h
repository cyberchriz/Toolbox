#ifndef ANGULAR_H
#define ANGULAR_H

#include "log.h"
#include <cmath>

// list of avaílable angular measures
enum AngularMeasure {
    RAD,        /// radians angle (full circle: 2*PI)
    DEG,        /// degree angle (full circle: 360)
    HOURS24,    /// time angle (full circle: 24)
    HOURS12,    /// time angle (full circle: 12)
    GON,        /// geodetic (full circle: 400)
    PERCENT,    /// percent (full circle: 100)
    NORMAL      /// normalized (full circle: 1.0)
};

// helper method for converting an angular value to a different unit
double angle(double value, AngularMeasure source_unit, AngularMeasure target_unit, bool exceed_full_circle = false) {
    static constexpr double_t PI = 3.1415926535897932384626433;
    static double_t full_circles = 0;

    switch (source_unit) {
        case AngularMeasure::RAD:
            full_circles = value / (2 * PI);
            break;
        case AngularMeasure::DEG:
            full_circles = value / 360;
            break;
        case AngularMeasure::HOURS24:
            full_circles = value / 24;
            break;
        case AngularMeasure::HOURS12:
            full_circles = value / 12;
            break;
        case AngularMeasure::GON:
            full_circles = value / 400;
            break;
        case AngularMeasure::PERCENT:
            full_circles = value / 100;
            break;
        case AngularMeasure::NORMAL:
            full_circles = value;
            break;
        default:
            Log::log(ERROR, "invalid source unit argument in function angle(double_t value, AngularMeasure source_unit, AngularMeasure target_unit)");
            break;
    }

    if (!exceed_full_circle) {
        full_circles = std::remainder(full_circles, 1);
    }
    
    switch (target_unit) {
        case AngularMeasure::RAD:
            return full_circles * (2 * PI);
        case AngularMeasure::DEG:
            return full_circles * 360;
        case AngularMeasure::HOURS24:
            return full_circles * 24;
        case AngularMeasure::HOURS12:
            return full_circles * 12;
        case AngularMeasure::GON:
            return full_circles * 400;
        case AngularMeasure::PERCENT:
            return full_circles * 100;
        case AngularMeasure::NORMAL:
            return full_circles;
        default:
            Log::log(ERROR, "invalid target unit argument in function angle(double_t value, AngularMeasure source_unit, AngularMeasure target_unit)");
    }

    return NAN;
}

#endif