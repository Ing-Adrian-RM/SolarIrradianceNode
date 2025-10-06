///////////////////////////////////////////////////////////////////////////////
// Display Module
// Description: This module manages the TFT display.
// Author: Adrian Rodriguez Murillo
///////////////////////////////////////////////////////////////////////////////

#include "config.h"

///////////////////////////////////////////////////////////////////////////////
// initialize_display: Initialize TFT display
///////////////////////////////////////////////////////////////////////////////
void initialize_display()
{
    display.begin();
    display.setContrast(255);
    display.setPowerSave(0);
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);
    display.drawStr(0, 30, "Display initialized");
    display.sendBuffer();
    delay(1000);
}

///////////////////////////////////////////////////////////////////////////////
// drawStatusScreen: Display system status
///////////////////////////////////////////////////////////////////////////////
void drawStatusScreen()
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);
    display.drawStr(0, 10, "    SYSTEM STATUS");

    // WiFi
    display.setCursor(0, 25);
    display.print("WiFi: ");
    display.print(getWiFiStatus());

    // Time & date (if NTP active and synced)
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    if (t && t->tm_year > 70)
    {
        char timestamp[20];
        char datestamp[20];

        strftime(timestamp, sizeof(timestamp), "%H:%M:%S", t);
        strftime(datestamp, sizeof(datestamp), "%d/%m/%y", t);

        display.setCursor(0, 40);
        display.print("Time: ");
        display.print(timestamp);

        display.setCursor(0, 55);
        display.print("Date: ");
        display.print(datestamp);
    }
    else
    {
        display.setCursor(0, 40);
        display.print("Time: --:--:--");
        display.setCursor(0, 55);
        display.print("Date: --/--/--");
    }

    display.sendBuffer();
}

///////////////////////////////////////////////////////////////////////////////
// drawPanelDataScreen: Display data from solar panels
///////////////////////////////////////////////////////////////////////////////
void drawPanelDataScreen(int startIndex)
{

    display.clearBuffer();
    display.setFont(u8g2_font_5x8_tf);
    char buf[32];

    int y = 10;
    int i = 0;
    for (SOLAR_CELL_LIST_PTR ptr = panel_list; ptr != NULL && i < startIndex + 2; ptr = ptr->next, i++)
    {
        if (i < startIndex)
            continue;
        snprintf(buf, sizeof(buf), "    Isc: %.2f mA", ptr->panel->Isc);
        display.setCursor(0, y);
        display.print(buf);
        y += 10;
        snprintf(buf, sizeof(buf), "P%d  G:   %.2f W/m2", i + 1, ptr->panel->Irradiance);
        display.setCursor(0, y);
        display.print(buf);
        y += 10;
        snprintf(buf, sizeof(buf), "    T:   %.1f C", ptr->panel->Temperature);
        display.setCursor(0, y);
        display.print(buf);
        y += 12;
    }
    display.sendBuffer();
}

///////////////////////////////////////////////////////////////////////////////
// drawAverageDataScreen: Display average data from all panels
///////////////////////////////////////////////////////////////////////////////
void drawAverageDataScreen()
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);

    display.drawStr(0, 10, "    AVERAGE DATA");
    display.setCursor(0, 30);
    display.printf("Isc:  %.2f mA", panel_avg.Isc);
    display.setCursor(0, 45);
    display.printf("G:    %.2f W/m2", panel_avg.Irradiance);
    display.setCursor(0, 60);
    display.printf("Temp: %.2f C", panel_avg.Temperature);
    display.sendBuffer();
}

///////////////////////////////////////////////////////////////////////////////
// drawTransmissionScreen: Display transmission status
///////////////////////////////////////////////////////////////////////////////
void drawTransmissionScreen()
{
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);

    display.drawStr(0, 10, "    TX BUFFER");
    display.setCursor(0, 30);
    display.print("Packets ready: ");
    display.print(buffer_ready ? "YES" : "NO");
    display.setCursor(0, 45);
    display.printf("RSSI: %d dBm", rssi);
    display.setCursor(0, 60);
    display.printf("SNR:  %.1f dB", snr);
    display.sendBuffer();
}

///////////////////////////////////////////////////////////////////////////////
// update_display: Update TFT display based on screen_id
///////////////////////////////////////////////////////////////////////////////
void update_display()
{
    unsigned long now = millis();
    if (now - last_screen_change >= SCREEN_INTERVAL)
    {
        current_screen = (current_screen + 1) % 6;
        last_screen_change = now;
    }

    switch (current_screen)
    {
    case 0:
        drawStatusScreen();
        break;
    case 1:
        drawPanelDataScreen(0);
        break;
    case 2:
        drawPanelDataScreen(2);
        break;
    case 3:
        drawPanelDataScreen(4);
        break;
    case 4:
        drawAverageDataScreen();
        break;
    case 5:
        drawTransmissionScreen();
        break;
    }
}
