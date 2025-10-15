///////////////////////////////////////////////////////////////////////////////
// Communication Module
// Description: This module handles LoRa communication and data uploads.
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// initialize_communication: Initialize LoRa module
///////////////////////////////////////////////////////////////////////////////
void initialize_communication()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi conectado!");
    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Configure NTP; -6 for CST (Costa Rica)

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(LORA_FREQ_HZ))
    {
        Serial.println("Error: Failed to initialize LoRa");
        while (1)
            ;
    }
    else
    {
        LoRa.setSignalBandwidth(LORA_BW_HZ);
        LoRa.setSpreadingFactor(LORA_SF);
        LoRa.setCodingRate4(LORA_CR_DEN);
        LoRa.enableCrc();
        Serial.println("LoRa initialized successfully");
    }
}

///////////////////////////////////////////////////////////////////////////////
// getWiFiStatus: Return WiFi connection status
///////////////////////////////////////////////////////////////////////////////
String getWiFiStatus()
{
    if (WiFi.status() == WL_CONNECTED)
        return "Connected";
    return "Disconnected";
}

///////////////////////////////////////////////////////////////////////////////
// handle_lora_requests: Listen for gateway requests and send data
///////////////////////////////////////////////////////////////////////////////
void handle_lora_requests()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        rssi = LoRa.packetRssi();
        snr = LoRa.packetSnr();
        Serial.println("Received packet with RSSI: " + String(rssi) + " dBm, SNR: " + String(snr) + " dB");
        String request = "";
        int byteIndex = 0;
        while (LoRa.available())
        {
            char c = (char)LoRa.read();
            if (byteIndex >= 4)
            {
                request += c;
            }
            byteIndex++;
        }

        Serial.println("Received via LoRa: " + request);
        if (request == "NODE1")
        {
            if (buffer_ready)
            {
                LoRa.beginPacket();
                LoRa.write((const uint8_t *)tx_buffer, strlen(tx_buffer));
                LoRa.endPacket();
                Serial.println("Sent via LoRa: " + String(tx_buffer));
                buffer_ready = false;
            }
            else
            {
                const char no_data[] = "NO_DATA";
                LoRa.beginPacket();
                LoRa.write((const uint8_t *)no_data, strlen(no_data));
                LoRa.endPacket();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// upload_to_thinkspeak_15sec: Upload data to Thingspeak
///////////////////////////////////////////////////////////////////////////////
void upload_to_thinkspeak()
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        String response = http.getString();
        Serial.println("Response from ThingSpeak: " + response);
    }
    else
    {
        Serial.println("Error on sending request to ThingSpeak: " + String(httpCode));
    }
    http.end();
}
