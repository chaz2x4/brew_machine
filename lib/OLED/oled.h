// OLED.h

#ifndef _OLED_h
#define _OLED_h

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

class OLED {
public:
    void start();
    void buttonTest();
    void clear();
    void event();
    void refresh();
    void changeMode();
private:
};

#endif

