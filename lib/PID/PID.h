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

    double *input;
    double *output;
    double *setpoint;

    double lastInput;
    double outputSum;

    double cycleTime = 200; //how long one cycle window lasts
public:
    PID(double *input, double *output, double *setpoint);
    void tune(double kp, double ki, double kd);
    void compute();
};

#endif