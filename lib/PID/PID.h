// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define HEATER_POWER 1370 // 1370 watt heating power
#define DUTY_PERIOD 500 //500ms cycle time manage PWM;

#define PWM_FREQUENCY 2048
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 10

class PID {
private:
    float kp = 0.0; // Proportional gain
    float ku = 0.0; // Ultimate Gain = Kp at neutral stability
    ulong tu = 0; // Neutral stability period
    ulong lastTime = 0; //time the last oscillation started
    ulong periodTime = 0; //period of last oscillation

    float errSum;
    float lastTemp;
    float lastErr;

    ulong fullDutyCycle = -1;
    ulong lastCycleTime = -1;
public:
    void initialize(int pwm_pin, float targetTemp, float actualTemp);
    void compute(float targetTemp, float actualTemp);
};

#endif