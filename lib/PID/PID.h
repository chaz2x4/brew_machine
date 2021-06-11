// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>

class PID {
private:
    double kp = 2;     // Proportional gain
    double ki = 5;   // Integral gain
    double kd = 1;      // Derivative gain

    ulong lastTime;
    ulong sampleTime;

    double *currentTemp;
    double *onTime;
    double *targetTemp;

    double lastInput;
    double outputSum;

    double cycleRunTime;
public:
    PID(double *input, double *output, double *setpoint, ulong time);
    void tune(double kp, double ki, double kd);
    void compute();
    void setCycleTime(double time);
};

#endif