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

#define HEATER_PIN 13
#define STEAM_PIN 27

#define RREF 430
#define DEFAULT_BREW_TEMP 95.0
#define DEFAULT_STEAM_TEMP 145.0
#define DEFAULT_OFFSET 10.0
#define EMERGENCY_SHUTOFF_TEMP 170.0

enum mode{brew, steam};

class GCP {
private:
    Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(A5);
    mode currentMode;

    double actualTemp;
    double targetTemp = DEFAULT_BREW_TEMP;
    double targetSteamTemp = DEFAULT_STEAM_TEMP;
    double tempOffset = DEFAULT_OFFSET;
    
    double maxBrewTemp = 100.0;
    double minBrewTemp = 75.0;

    double maxSteamTemp = 165.0;
    double minSteamTemp = 140.0;

    double brew_output;
    double steam_output;

    ulong cycleStartTime;
    ulong cycleRunTime;

    PID brewTempManager = PID(&actualTemp, &brew_output, &targetTemp, 5000);
    PID steamTempManager = PID(&actualTemp, &steam_output, &targetSteamTemp, 5000);

public:
    GCP();
    GCP(double, double, double);
    ~GCP();
    mode getCurrentMode();
    void setMode(mode);
    void incrementTemp();
    void decrementTemp();
    double getTargetTemp();
    double getTargetSteamTemp();
    double getActualTemp(); //returns current temperature value
    double getTempOffset();
    void setTargetTemp(double); // Sets temperature to control to (setpoint)
    void setTargetSteamTemp(double);
    void setTempOffset(double);
    void update();
};
#endif