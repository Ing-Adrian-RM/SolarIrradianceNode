///////////////////////////////////////////////////////////////////////////////
// processing_module.h: Header file for processing_module.cpp
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef PROCESSING_MODULE_H
#define PROCESSING_MODULE_H

#include <config.h>

void initialize_panels_structs();
SOLAR_CELL_LIST_PTR insert_panel(SOLAR_CELL_LIST_PTR list, solar_cell *panel);
float Isc_to_irradiance(float current_mA, float temperature_C);
void panel_data_average();
void transmission_buffer();
void thingspeak_url_5min_cal();
void thingspeak_url_15sec();
void calibration_average();

#endif
