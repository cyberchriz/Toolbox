#ifndef ANGULAR_H
#define ANGULAR_H

#include <cmath>
#include <log.h>

// list of avaílable angular measures
enum AngularUnit {
	RAD,        /// radians angle (full circle: 2*PI)
	DEG,        /// degree angle (full circle: 360)
	HOURS24,    /// time angle (full circle: 24)
	HOURS12,    /// time angle (full circle: 12)
	GON,        /// geodetic (full circle: 400)
	PERCENT,    /// percent (full circle: 100)
	NORMAL      /// normalized (full circle: 1.0)
};

// helper method for converting an angular value to a different unit
double convert_angle(double value, AngularUnit source_unit, AngularUnit target_unit, bool exceed_full_circle = true) {
	static constexpr double_t PI = 3.1415926535897932384626433;
	static double_t full_circles = 0;

	switch (source_unit) {
	case AngularUnit::RAD:
		full_circles = value / (2 * PI);
		break;
	case AngularUnit::DEG:
		full_circles = value / 360;
		break;
	case AngularUnit::HOURS24:
		full_circles = value / 24;
		break;
	case AngularUnit::HOURS12:
		full_circles = value / 12;
		break;
	case AngularUnit::GON:
		full_circles = value / 400;
		break;
	case AngularUnit::PERCENT:
		full_circles = value / 100;
		break;
	case AngularUnit::NORMAL:
		full_circles = value;
		break;
	default:
		Log::error("invalid source unit argument in function angle(double_t value, AngularUnit source_unit, AngularUnit target_unit)");
		break;
	}

	if (!exceed_full_circle) {
		full_circles = std::remainder(full_circles, 1);
	}

	switch (target_unit) {
	case AngularUnit::RAD:
		return full_circles * (2 * PI);
	case AngularUnit::DEG:
		return full_circles * 360;
	case AngularUnit::HOURS24:
		return full_circles * 24;
	case AngularUnit::HOURS12:
		return full_circles * 12;
	case AngularUnit::GON:
		return full_circles * 400;
	case AngularUnit::PERCENT:
		return full_circles * 100;
	case AngularUnit::NORMAL:
		return full_circles;
	default:
		Log::error("invalid target unit argument in function angle(double_t value, AngularUnit source_unit, AngularUnit target_unit)");
	}

	return NAN;
}

#endif