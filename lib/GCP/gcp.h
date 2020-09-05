// gcp.h
#ifndef _GCP_h
#define _GCP_h
/*
    Heat Theory:
        1) Turn on machine
        2) Heat water to brew temperature, or steam temperature if that switch is on
        3) Turn on lights when at specific temperatures
    Only input should be steam switch, which moves PID temp to 155 C
    For now leave pump switch manually on the coffee switch

    Pre-infusion Theory

*/

#include <EEPROM.h>
#include "PID.h"

#define ON true;
#define OFF false;
#define DEFAULT_TARGET_TEMP 95.0
#define DEFAULT_ACTUAL_TEMP 15.0
#define MAX_BREW_TEMP 96.0
#define MIN_BREW_TEMP 90.0
#define MAX_STEAM_TEMP 155.0
#define MIN_STEAM_TEMP 145.0

class GCP {
private:
    float targetTemp = DEFAULT_TARGET_TEMP; //95 celcius
    float actualTemp = DEFAULT_ACTUAL_TEMP; // #4 coffee thermostat; turns off heating element when = targetBrewTemp

    /* 
        Manual Switch Status
        There's a chance I will not be able to detect the status of these
        Hopefully with a powerful enough resistor then this doesn't matter
        It might be possible to detect status through thermocouple
    */
    bool power_switch; // #1 on/off switch
    bool coffee_switch; // #9 coffee switch; turns on pump
    bool steam_switch; // #7 coffee/steam switch (off when the other is on)
    bool pump_switch; // #10 motor operated water pump; turns on with brew switch

    /* Temperature controlled switches */
    bool heating_switch; // #3 thermal cut off?

    bool power_light = OFF; // #2 on/off light; turns on with power switch
    bool brew_light = OFF; // #11 pilot light; at the brew temperature
    bool steam_light = OFF;  //at the steam temperature

    // #6 is the heating element
    // #8 is the solenoid valve

    PID temperatureManager;

    void init(float targetTemp);

public:
    GCP();
    GCP(float targetTemp);
    ~GCP();
    void incrementTemp();
    void decrementTemp();
    float getTargetTemp();
    float getActualTemp(); //returns current temperature value
    bool isSteamReady();
    bool isBrewReady();
    bool isPowerOn();
    float getPX(); //returns current pressure value
    void setTargetTemp(float temp); // Sets temperature to control to (setpoint)
    void setTargetTemp(float temp, float minTemp, float maxTemp); // Sets temperature to control to (setpoint), wrap around with a set minimum / maximum temperature
    void update();
};
#endif
