///////////////////////////////////////////////////////////////////////////////
// display_module.h: Header file for display_module.cpp
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include <config.h>

void initialize_display();
void drawStatusScreen();
void drawPanelDataScreen(int startIndex);
void drawAverageDataScreen();
void drawTransmissionScreen();
void update_display();


#endif
