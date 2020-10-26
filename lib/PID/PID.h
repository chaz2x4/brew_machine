// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
#define MAX_HEATER_POWER 1370 // 1370 watt heating power
#define DUTY_PERIOD 500 //500ms cycle time manage PWM;

#define PWM_FREQUENCY 2048
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 10

#define MAX_TERM_VALUE 1000

class PID {
private:
    double kp = 25;     // Proportional gain
    double ki = 1000;   // Integral gain
    double kd = 9;      // Derivative gain

    double P = 0.0;
    double I = 0.0;
    double D = 0.0;

    double lastErr;
public:
    void initialize(int pwm_pin, double targetTemp, double actualTemp);
    void tune(double kp, double ki, double kd);
    void compute(double targetTemp, double actualTemp);
};

#endif