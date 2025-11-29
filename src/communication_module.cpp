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
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi conectado!");
    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Configure NTP; -6 for CST (Costa Rica)

    // Initialize LoRa module
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(LORA_FREQ))
    {
        Serial.println("Error: Failed to initialize LoRa");
        while (1)
            ;
    }
    else
    {
        LoRa.setSignalBandwidth(LORA_BW);     // 500 kHz
        LoRa.setSpreadingFactor(LORA_SF);     // SF10
        LoRa.setCodingRate4(LORA_CR);         // 4/5
        LoRa.enableCrc();                     // CRC ON   
        LoRa.setSyncWord(LORA_SYNC_WORD);     // 0x12
        LoRa.setPreambleLength(8);            // igual a RH por defecto
        LoRa.setTxPower(TX_POWER_DBM, PA_OUTPUT_PA_BOOST_PIN); // PA_BOOST
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
// send_lora: Create the package header (4 bytes) + payload
// Inputs: to -> Rx LoRa address
//         payload -> Buffer to send
///////////////////////////////////////////////////////////////////////////////
void send_lora(uint8_t to, const String &payload)
{
    LoRa.beginPacket();
    LoRa.write(to);            // TO
    LoRa.write(NODE_ADDR);     // FROM
    LoRa.write(g_msg_id++);    // ID
    LoRa.write((uint8_t)0x00); // FLAGS
    LoRa.print(payload);       // PAYLOAD ASCII
    LoRa.endPacket();
}

///////////////////////////////////////////////////////////////////////////////
// recv_lora: Receive the package and split the header's elements and payload
// Inputs: to -> Rx LoRa address
//         from -> Tx LoRa address
//         id -> Package ID
//         payload -> Buffer to receive
//         timeout_ms -> Wait time 
///////////////////////////////////////////////////////////////////////////////
bool recv_lora(uint8_t &to, uint8_t &from, uint8_t &id, uint8_t &flags, String &payload)
{
    uint32_t t0 = millis();
    while (millis() - t0 < timeout_ms)
    {
        int pktLen = LoRa.parsePacket();
        if (pktLen >= 5)
        {
            to = (uint8_t)LoRa.read();
            from = (uint8_t)LoRa.read();
            id = (uint8_t)LoRa.read();
            flags = (uint8_t)LoRa.read();

            payload.reserve(pktLen - 4);
            payload = "";
            while (LoRa.available())
            {
                payload += (char)LoRa.read();
            }

            // Debug RX
            Serial.printf("RX  TO:%u FROM:%u ID:%u FL:%u | RSSI:%d SNR:%.2f | '%s'\n",
                          to, from, id, flags, LoRa.packetRssi(), LoRa.packetSnr(), payload.c_str());
            return true;
        }
        delay(1);
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// handle_lora_requests: Listen for gateway requests and send data
///////////////////////////////////////////////////////////////////////////////
void handle_lora_requests()
{
    uint8_t to, from, id, flags;
    String payload;

    // Wait data request from Gateway
    if (recv_lora(to, from, id, flags, payload))
    {
        rssi = LoRa.packetRssi();
        snr = LoRa.packetSnr();
        if (to == NODE_ADDR)
        {
            String expected = "Data!" + String(NODE_ADDR);
            if (payload == expected)
            {

                // ACK to check command received on TTGO
                send_lora(from, "ok");
                Serial.println("TX: ACK send");
                delay(120);

                // Send payload
                char buffer[BUFFER_SIZE];
                if (buffer_ready)
                {
                    snprintf(buffer, BUFFER_SIZE, "%s", tx_buffer);
                    buffer_ready = false;
                }
                else
                {
                    snprintf(buffer, BUFFER_SIZE, "%s", "NO_DATA");
                }
                send_lora(from, String(buffer));
                Serial.printf("TX: data '%s'\n", buffer);

                // ACK to check data was received on Gateway
                uint32_t t0 = millis();
                while (millis() - t0 < timeout_ms)
                {
                    uint8_t t2, f2, i2, fl2;
                    String p2;
                    if (recv_lora(t2, f2, i2, fl2, p2) && t2 == NODE_ADDR && p2 == "ok")
                    {
                        Serial.println("RX: ACK received.");
                        break;
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// upload_to_thinkspeak_15sec: Upload data to Thingspeak
///////////////////////////////////////////////////////////////////////////////
void upload_to_thingspeak()
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
