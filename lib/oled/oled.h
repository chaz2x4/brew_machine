// OLED.h

#ifndef _OLED_h
#define _OLED_h

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "gcp.h"

#define BUTTON_A 15 //pin 15; cycle modes; increment temeperature
#define BUTTON_B 32 //pin 32; cycle modes; decrement temperature 
#define BUTTON_C 14 //pin 14; press and hold on brew screen to change temperature

#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_I2C_ADDRESS 0x3C

class OLED {
public:
    OLED();
    ~OLED();
    void start(GCP*);
    void eventListener();
    void refresh(ulong);
    void changeMode();
    TempMode getCurrentMode();
    void incrementTemp();
    void decrementTemp();
private:
    Adafruit_SSD1306 display;
    GCP *gcp;
    
    TempMode current_mode;
    ulong real_time;
    ulong time_last_button;   //records the last time a button was pressed
    ulong last_time;         // records time of event
    ulong downtime;         // records when button was pressed and held for hold functionality
    bool is_editable;        // is screen on the setTemperature method
    bool flash;             // if true screen is on alternate screen
    int button_state[3];     // records button state HIGH is unpressed
    int last_button_state[3]; // checks if button was pressed by comparing state to button_state
    const int kScreenTimeout; // amount of milliseconds before screen goes blank
    const int kTriggerTime;  // amount of time to hold button for settings change
    bool checkedIfTimedout();
};

#endif