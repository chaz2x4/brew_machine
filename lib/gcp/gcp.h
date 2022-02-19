// GCP.h

#ifndef _GCP_h
#define _GCP_h

#include <Adafruit_MAX31865.h>
#include <ACS712.h>
#include <RBDdimmer.h>
#include <EEPROM.h>
#include <PID_v1.h>

/* PINS */
#define HEATER_PIN 13
#define STEAM_PIN 12
#define PUMP_PIN 27
#define ZERO_CROSS_PIN 33
#define THERMOPROBE_PIN A5
#define CURRENT_PIN A4
#define TRANSDUCER_PIN A3

/* EEPROM ADDRESSES */
#define BREW_TEMP_ADDRESS 0
#define STEAM_TEMP_ADDRESS 8
#define OFFSET_ADDRESS 16
#define TUNING_ADDRESS 24

/* HARDWARE VERSIONS */
#define ACS_VERSION ACS712_30A
#define RREF 430

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
    double getTargetPressure();
    double getTargetTemp(String);
    double getPressure();
    double getActualTemp();
    double getCurrentTemp();
    String getOutput();
    String getTunings();
    void setTargetPressure(double);
    void setTargetTemp(String, double);
    void setTunings(double, double, double);
    bool isBrewing();
    ulong getBrewStartTime();
    ulong getBrewStopTime();
    void runBrewProfile(ulong);
    void refresh(ulong);
private:
    Adafruit_MAX31865 tempProbe;
    uint8_t probe_fault;
    ACS712 currentSensor;
    dimmerLamp pumpDimmer;
    const double kEmergencyShutoffTemp;
    const double kMaxBrewTemp;
    const double kMinBrewTemp;
    const double kMaxSteamTemp;
    const double kMinSteamTemp;
    const double kMaxOffset;
    const double kMinOffset;
    const int kWebsiteQueueSize;
    const ulong kWindowSize;
    const ulong kLogInterval;
    const int kPowerFrequency;
    const double kTranducerLimit; // In bars, ie, 1.2Mpa = 12 bars

    double current_temp;
    double temp_offset;
    double target_temp;
    double target_steam_temp;

    double current_pressure;
    double target_pressure;

    double brew_output;
    double steam_output;
    double pump_output;
    int last_brew_output;
    int last_steam_output;

    ulong window_start_time;
    ulong log_start_time;
    ulong brew_start_time;
    ulong brew_stop_time;
    ulong preinfusion_time;

    double temp_kp;
    double temp_ki;
    double temp_kd;

    double px_kp;
    double px_ki;
    double px_kd;

    Queue outputQueue;
    PID brewTempManager;
    PID steamTempManager;
    PID pumpPressureManager;

    int regulateOutput(double);
    void parseQueue(ulong);
    void loadParameters();
};
#endif