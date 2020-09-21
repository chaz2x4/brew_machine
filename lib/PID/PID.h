// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define HEATER_POWER 1370 // 1370 watt heating power
#define DUTY_CYCLE 500 //500ms cycle time manage PWM;

class PID {
private:
    float ku = 0.0; // Ultimate Gain = Kp at neutral stability
    int tu = 0; // Neutral stability period

    float errSum;
    float lastTemp;
    float lastErr;

    ulong lastCycleTime = -1;
    bool heater_status = true;

    void PWM(float onPercent);
public:
    void initialize(float targetTemp, float actualTemp);
    void compute(float targetTemp, float actualTemp);
    bool isHeaterRunning();
};

#endif