// PID.h

#ifndef _PID_h
#define _PID_h

#include <Arduino.h>

class PID {
private:
    float kp = 0.5;
    float ki = 0;
    float kd = 0;
    int runTime = 500; //half a second run time

    float iTerm;
    float lastTime;
    float lastTemp;
    float outputV;

public:
    void compute(int targetTemp, float actualTemp);
    void setTunings(float kp, float ki, float kd);
    void setRuntime(int runTime);
};

#endif

