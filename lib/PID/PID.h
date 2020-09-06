// PID.h

#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define HEATER_POWER 1370 // 1370 watt heating power
#define BOILER_SIZE .1 // 3.5oz = 100 ml
#define EFFICIENCY .60 // 90% efficiency
#define CP_WATER 4186.0 // Specific Heat Capacity of Water

class PID {
private:
    float kp = 0.0;
    float ki = 0.0;
    float kd = 0.0;
    float ku = 0.0;

    int ti = 0; 
    int td = 0;
    int tu = 0;

    int runTime;
    float errSum;
    float lastTime;
    float lastTemp;
    float outputV;

    bool heating_element;
public:
    void initialize(float targetTemp, float actualTemp);
    void compute(float targetTemp, float actualTemp);
    void setTunings(float ku, int tu);
    void setRuntime(int runTime);
    bool runHeater();
};

#endif

