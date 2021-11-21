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
#include <PID_v1.h>
#include <sstream>
#include <string>

#define ON HIGH
#define OFF LOW

#define HEATER_PIN 13
#define STEAM_PIN 27

#define RREF 430
#define DEFAULT_BREW_TEMP 96.0
#define DEFAULT_STEAM_TEMP 135.0
#define DEFAULT_OFFSET -6.0
#define EMERGENCY_SHUTOFF_TEMP 150.0
#define MAX_OFFSET 15
#define MIN_OFFSET -15

#define CYCLE_TIME 500

enum mode{brew, steam};

class GCP {
private:
    Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(A5);
    mode currentMode;

    void init(double, double, double);

    double actualTemp;
    double tempOffset = DEFAULT_OFFSET;
    double targetTemp = DEFAULT_BREW_TEMP;
    double targetSteamTemp = DEFAULT_STEAM_TEMP;
    
    double maxBrewTemp = 100.0;
    double minBrewTemp = 85.0;

    double maxSteamTemp = 140.0;
    double minSteamTemp = 120.0;

    double brew_output;
    double steam_output;

    ulong cycleStartTime;

    PID brewTempManager = PID(&actualTemp, &brew_output, &targetTemp, 25, 12, 3, P_ON_M, DIRECT);
    PID steamTempManager = PID(&actualTemp, &steam_output, &targetSteamTemp, 25, 12, 3, P_ON_M, DIRECT);

public:
    void start();
    mode getCurrentMode();
    void setMode(mode);
    void incrementTemp();
    void decrementTemp();
    double getTargetTemp();
    double getTargetSteamTemp();
    double getActualTemp(); //returns current temperature value
    double getTempOffset();
    String getOutput();
    String getTunings();
    void setTargetTemp(double); // Sets temperature to control to (setpoint)
    void setTargetSteamTemp(double);
    void setTempOffset(double);
    void setTunings(double, double, double);
    void refresh();
};
#endif