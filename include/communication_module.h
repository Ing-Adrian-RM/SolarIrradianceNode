///////////////////////////////////////////////////////////////////////////////
// communication_module.h: Header file for communication_module.cpp
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef COMMUNICATION_MODULE_H
#define COMMUNICATION_MODULE_H

#include <config.h>

void initialize_communication();
String getWiFiStatus();
void handle_lora_requests();
void upload_to_thinkspeak();

#endif
