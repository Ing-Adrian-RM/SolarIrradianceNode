///////////////////////////////////////////////////////////////////////////////
// sensors_module.h: Header file for sensors_module.cpp
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef SENSORS_MODULE_H
#define SENSORS_MODULE_H

#include <config.h>

void initialize_sensors();
void read_ina226_sensors();
void read_ads1115_sensors();

#endif
