// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>

class PID {
private:
    double kp = 5;     // Proportional gain
    double ki = 3;   // Integral gain
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
    PID(double*, double*, double*, ulong);
    void tune(double, double, double);
    void compute();
    void setCycleTime(double);
};

#endif