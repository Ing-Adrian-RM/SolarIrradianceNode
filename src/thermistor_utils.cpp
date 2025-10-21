///////////////////////////////////////////////////////////////////////////////
// Thermistor utility functions
// Description: Functions to convert voltage readings from ADS1115 to temperature in Celsius
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// voltage_to_resistance: Convert voltage reading to thermistor resistance
///////////////////////////////////////////////////////////////////////////////
float voltage_to_resistance(float vout)
{
    if (vout <= 0.0f)
        return 1e9f; // essentially infinite (open)
    if (vout >= VCC)
        return 1e-3f; // near zero (short)

    // Voltage divider formula: R_therm = R_pullup * (V_out / (V_in - V_out))
    return R_PULLUP * (vout / (VCC - vout));
}

///////////////////////////////////////////////////////////////////////////////
// resistance_to_celsius: Convert thermistor resistance to temperature (°C)
///////////////////////////////////////////////////////////////////////////////
float resistance_to_celsius(float rtherm)
{
    // Check for invalid resistance
    if (rtherm <= 0.0f)
        return -273.15f; // Absolute zero in Celsius
    
    // Beta parameter equation: 1/T = 1/T0 + (1/B) * ln(R/R0)
    float invT = (1.0f / T0_K) + (1.0f / BETA) * log(rtherm / R0);
    float T = 1.0f / invT;

    // Convert to Celsius
    return T - 273.15f;
}

///////////////////////////////////////////////////////////////////////////////
// resistance_to_celsius: Convert thermistor resistance to temperature (°C)
///////////////////////////////////////////////////////////////////////////////
float ads_raw_to_celsius(int16_t raw)
{
    float vout = (float)raw * ADS1115_LSB_GAIN_ONE;
    float rtherm = voltage_to_resistance(vout);
    float tempC = resistance_to_celsius(rtherm);
    return tempC;
}
