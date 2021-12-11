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

#define SCREEN_TIMEOUT 900000 //amount of milliseconds before screen goes blank
#define TRIGGER_TIME 1000 //amount of time to hold button for settings change

class OLED {
public:
    /* Internal to the OLED */
    void start(GCP* brew_machine); //Starts the display with the appropriate pins initialize
    void eventListener(); //Listens for button presses
    void refresh(); //refreshes screen every delay cycle

    /* Outputed to the WebServer */
    void incrementTemp();
    void decrementTemp();
private:
    Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire); //OLED screen
    bool flash = false; //controls which version of the OLED screen to display
    String currentMode = "brew";

    ulong timeLastButton = -1; //records the last time a button was pressed
    ulong lastTime = -1;
    ulong downTime = -1; //records when button was pressed and held for hold functionality

    int buttonState[3] = {HIGH, HIGH, HIGH};
    int lastButtonState[3] = {HIGH, HIGH, HIGH};

    bool timedout(); //turns screen off after a set amount of time
    bool isEditable = false; //is screen on the setTemperature method

    GCP *gcp;
    void changeMode(); //switches between brew and steam mode
};

#endif