// PID.h
#ifndef _Controller_h
#define _Controller_h

#include <Arduino.h>

class Controller {
private:
    double kp;     // Proportional gain
    double ki;   // Integral gain
    double kd;      // Derivative gain

    ulong lastTime;
    ulong sampleTime;

    double *currentTemp;
    double *onTime;
    double *targetTemp;

    double lastInput;
    double outputSum;

    double cycleRunTime;
public:
    Controller(double*, double*, double*, ulong);
    void tune(double, double, double);
    void compute();
    void setCycleTime(double);
    double* getTunings(double*);
};

#endif