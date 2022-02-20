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

typedef enum {C, F} temp_scale;
typedef struct Queue {
    int front, rear, capacity, count;
    typedef enum mode{brew, steam, offset};
    String *queue;

    temp_scale current_scale;
    ulong *times;
    double *temps;
    double *outputs[2];
    double *targets[3];

    Queue(int c, temp_scale s) {
        current_scale = s;
        front = rear = 0;
        capacity = c;
        times = new ulong[capacity];
        temps = new double[capacity];
        for(int i=0;i<2;i++){
            outputs[i] = new double[capacity];
        }
        for(int i=0;i<3;i++) {
            targets[i] = new double[capacity];
        }
    }

    ~Queue() { 
        delete[] times;
        delete[] temps;
        for(int i=0;i<2;i++) {
            delete[] outputs[i];
        }
        for(int i=0;i<3;i++) {
            delete[] targets[i];
        }
    }

    void push(
        ulong time, 
        double temp, 
        double brew_output, 
        double steam_output, 
        double brew_target, 
        double steam_target, 
        double offset_target
    ){
        if(count == capacity) pop();
        if(rear == capacity - 1) rear = -1;
        if(count == 0) {
            times[0] = time;
            temps[0] = temp;
            outputs[brew][0] = brew_output;
            outputs[steam][0] = steam_output;
            targets[brew][0] = brew_target;
            targets[steam][0] = steam_target;
            targets[offset][0] = offset_target;
        }
        else {
            ++rear;
            times[rear] = time;
            temps[rear] = temp;
            outputs[0][rear] = brew_output;
            outputs[1][rear] = steam_output;
            targets[0][rear] = brew_target;
            targets[1][rear] = steam_target;
            targets[2][rear] = offset_target;
        }
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

    String at(int i) {
        String results;
        results += "{ \"time\": ";
        results += times[i];
        results += ", \"temperature\": ";
        results += temps[i];
        results += ", \"outputs\": { \"brew\": ";
        results += outputs[brew][i];
        results += ", \"steam\": ";
        results += outputs[steam][i];
        results += "}, \"targets\": { \"brew\": ";
        results += targets[brew][i];
        results += ", \"steam\": ";
        results += targets[steam][i];
        results += ", \"offset\": ";
        results += targets[offset][i];
        results += " }}";
        return results;
    }

    String getScale(){
        if(current_scale == C) return "C";
        else return "F";
    }

    void setScale(temp_scale scale) {
        current_scale = scale;
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
    void changeScale(String);
    bool isBrewing();
    ulong getBrewStartTime();
    ulong getBrewStopTime();
    void runBrewProfile(ulong);
    void refresh(ulong);
private:
    Adafruit_MAX31865 tempProbe;
    ACS712 currentSensor;
    dimmerLamp pumpDimmer;
    const double kEmergencyShutoffTemp;
    const double kMaxBrewTemp;
    const double kMinBrewTemp;
    const double kMaxSteamTemp;
    const double kMinSteamTemp;
    const double kMaxOffset;
    const double kMinOffset;
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
    void loadParameters();
};
#endif
