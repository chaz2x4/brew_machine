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
    float lastKp = 0.0; // Used to compare oscillations to determine ku
    ulong period = 0; // Period of oscillation
    ulong lastPeriod = 0; // Used to compare oscillations to determine tu
    ulong lastTime;
    
    float ku = 0.0; // Ultimate Gain = Kp at neutral stability
    ulong tu = 0; // Neutral stability period

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