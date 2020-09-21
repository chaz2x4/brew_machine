// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define HEATER_POWER 1370 // 1370 watt heating power
#define DUTY_CYCLE 500 //500ms cycle time manage PWM;

class PID {
private:
    float kp = 0.0; // Proportional gain
    float ku = 0.0; // Ultimate Gain = Kp at neutral stability
    ulong tu = -1; // Neutral stability period
    ulong lastPeriodTime = -1; //time the last ku oscillation started

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