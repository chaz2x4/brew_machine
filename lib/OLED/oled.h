// OLED.h

#ifndef _OLED_h
#define _OLED_h

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GCP.h"

#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

enum mode{brew, steam};

class OLED : public GCP {
private:
    Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
    mode currentMode = brew;
    bool isEditable = false;
    void wait(int delay, char* text1, float var1, char* text2, float var2);
public:
    void start();
    void eventListener();
    void refresh();
    void changeMode();
};

#endif

