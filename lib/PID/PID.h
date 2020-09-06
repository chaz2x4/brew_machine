// PID.h

#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define HEATER_POWER 1370 // 1370 watt heating power
#define BOILER_SIZE .1 // 3.5oz = 100 ml
#define EFFICIENCY .60 // 60% efficiency
#define CP_WATER 4186.0 // Specific Heat Capacity of Water

#define DUTY_CYCLE 4000 //4 seconds cycle to manage PWM; keep it slow to avoid SSR zero crossing errors

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
    ulong lastTime;
    ulong lastTu;
    float errSum;
    float lastTemp;
    float lastErr;
    float outputV;

    ulong lastCycleTime = -1;
    bool heater_status = true;

    void PWM(float onPercent);
public:
    void initialize(float targetTemp, float actualTemp);
    void compute(float targetTemp, float actualTemp);
    bool isHeaterRunning();
};

#endif

