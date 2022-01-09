// OLED.h

#ifndef _OLED_h
#define _OLED_h

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GCP.h"

#define BUTTON_A 15 //pin 15; cycle modes; increment temeperature
#define BUTTON_B 32 //pin 32; cycle modes; decrement temperature 
#define BUTTON_C 14 //pin 14; press and hold on brew screen to change temperature

class OLED {
public:
    OLED();
    ~OLED();
    void start(GCP*);
    void eventListener();
    void refresh();
    void changeMode();
    String getCurrentMode();
    void incrementTemp();
    void decrementTemp();
private:
    Adafruit_SSD1306 display;
    GCP *gcp;
    String currentMode;
    
    ulong timeLastButton;   //records the last time a button was pressed
    ulong lastTime;         // records time of event
    ulong downTime;         // records when button was pressed and held for hold functionality
    bool isEditable;        // is screen on the setTemperature method
    bool flash;             // if true screen is on alternate screen
    int buttonState[3];     // records button state HIGH is unpressed
    int lastButtonState[3]; // checks if button was pressed by comparing state to buttonState
    const int screenTimeout; // amount of milliseconds before screen goes blank
    const int triggerTime;  // amount of time to hold button for settings change
    bool checkedIfTimedout();
};

#endif