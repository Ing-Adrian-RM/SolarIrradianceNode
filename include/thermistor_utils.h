///////////////////////////////////////////////////////////////////////////////
// thermistor_utils.h: Utility functions and constants for thermistor calculations
///////////////////////////////////////////////////////////////////////////////

#ifndef THERMISTOR_UTILS_H
#define THERMISTOR_UTILS_H

#include <config.h>

float voltage_to_resistance(float vout);
float resistance_to_celsius(float rtherm);
float ads_raw_to_celsius(int16_t raw);

#endif