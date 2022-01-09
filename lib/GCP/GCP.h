// GCP.h

#ifndef _GCP_h
#define _GCP_h

#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include <PID_v1.h>
// #include <PID_AutoTune_v0.h>

#define HEATER_PIN 13
#define STEAM_PIN 27
#define THERMOPROBE_PIN A5
#define RREF 430

#define BREW_TEMP_ADDRESS 0
#define STEAM_TEMP_ADDRESS 8
#define OFFSET_ADDRESS 16
#define TUNING_ADDRESS 24

struct Queue {
    int front, rear, capacity, count;
    String *queue;

    Queue(int c) {
        front = rear = 0;
        capacity = c + 1;
        queue = new String[capacity];
    }

    ~Queue() { delete[] queue; }

    void push(String data){
        if(count == capacity) pop();
        if(rear == capacity - 1) rear = -1;
        if(count == 0) queue[0] = data;
        else queue[++rear] = data;
        count++;
    }

    void pop() {
        front++;
        if(front == capacity) front = 0;
        count--;
    }

    int size() {
        return count;
    }

    String at(int i){
        return queue[i];
    }
};

class GCP {
public:
    GCP();
    ~GCP();
    void start();
    void incrementTemp(String);
    void decrementTemp(String);
    double getTargetTemp(String);
    double getActualTemp();
    double getCurrentTemp();
    String getOutput();
    String getTunings();
    double getCurrentTimer(ulong);
    void setTargetTemp(String, double);
    void setTunings(double, double, double);
    void startTimer(ulong);
    void stopTimer();
    void refresh(ulong);
    // void autoTune();
    // void cancelAutoTune();
private:
    Adafruit_MAX31865 tempProbe;
    const double emergencyShutoffTemp;
    const double maxBrewTemp;
    const double minBrewTemp;
    const double maxSteamTemp;
    const double minSteamTemp;
    const double maxOffset;
    const double minOffset;
    const int websiteQueueSize;
    const ulong windowSize;
    const ulong logInterval;
    const int powerFrequency;

    double currentTemp;
    double tempOffset;
    double targetTemp;
    double targetSteamTemp;

    double brew_output;
    double steam_output;
    int lastBrewOutput;
    int lastSteamOutput;

    ulong windowStartTime;
    ulong logStartTime;
    ulong timerStartTime;
    ulong lastTime;

    bool brewSwitchOn;

    double Kp;
    double Ki;
    double Kd;

    Queue outputQueue;
    PID brewTempManager;
    PID steamTempManager;

    // PID_ATune autoTuner;
    // bool isTuned;

    double sensedCurrent();
    int regulateOutput(double);
    void parseQueue(ulong);
    void loadParameters();
};
#endif