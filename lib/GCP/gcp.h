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

*/

#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include "PID.h"

#define ON HIGH
#define OFF LOW

#define HEATER_PIN 27
#define STEAM_PIN 13

#define RREF 430

class GCP {
private:
    Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(A5);

    double actualTemp;
    double targetTemp = 100.0;
    double targetSteamTemp = 155.0;
    double pressure = 0.0; 
    
    double maxBrewTemp = 100.0;
    double minBrewTemp = 75.0;
    double emergencyShutoffTemp = 165.0;

    void init(double targetTemp);

    double brew_output;
    double steam_output;

    PID brewTempManager = PID(&actualTemp, &brew_output, &targetTemp);
    PID steamTempManager = PID(&actualTemp, &steam_output, &targetSteamTemp);
    
public:
    GCP();
    GCP(double targetTemp);
    ~GCP();
    void init();
    void incrementTemp();
    void decrementTemp();
    double getTargetTemp();
    double getActualTemp(); //returns current temperature value
    bool isSteamReady();
    bool isBrewReady();
    bool isPowerOn();
    double getPX(); //returns current pressure value
    void setTargetTemp(double temp); // Sets temperature to control to (setpoint)
    void update();
};
#endif
