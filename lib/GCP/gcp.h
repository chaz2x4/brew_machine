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

#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include "PID.h"

#define ON HIGH
#define OFF LOW

#define DEFAULT_TARGET_TEMP 95.0
#define TARGET_STEAM_TEMP 155.0
#define MAX_BREW_TEMP 100.0
#define MIN_BREW_TEMP 85.0

#define EMERGENCY_SHUTOFF_TEMP 165.0

#define HEATER_PIN 27
#define STEAM_PIN 13

#define PWM_FREQUENCY 10 // PWM Frequency of 10Hz
#define PWM_RESOLUTION 10 // Resolution of 10 bits
#define PWM_DUTY_CYCLE 1023 // 10-bit resolution results in a duty cycle of 0 - 1023
#define PWM_BREW_CHANNEL 0 // Different PWM channels to control different relays
#define PWM_STEAM_CHANNEL 1

#define RREF 430

class GCP {
private:
    PID brewTempManager;
    PID steamTempManager;
    Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(A5);

    float targetTemp = DEFAULT_TARGET_TEMP; //95 celcius
    float pressure = 0.0; 

    void init(float targetTemp);
public:
    GCP();
    GCP(float targetTemp);
    ~GCP();
    void init();
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
