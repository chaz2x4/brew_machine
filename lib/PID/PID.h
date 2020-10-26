// PID.h
#ifndef _PID_h
#define _PID_h

#include <Arduino.h>
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
    void tune(double kp, double ki, double kd);
    double compute(double targetTemp, double actualTemp);
};

#endif