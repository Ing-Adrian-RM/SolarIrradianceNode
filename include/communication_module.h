///////////////////////////////////////////////////////////////////////////////
// communication_module.h: Header file for communication_module.cpp
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#ifndef COMMUNICATION_MODULE_H
#define COMMUNICATION_MODULE_H

#include <config.h>

void initialize_communication();
String getWiFiStatus();
void send_lora(uint8_t to, const String &payload);
bool recv_lora(uint8_t &to, uint8_t &from, uint8_t &id, uint8_t &flags, String &payload);
void handle_lora_requests();
void upload_to_thinkspeak();

#endif
