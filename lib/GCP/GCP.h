// gcp.h
#ifndef _GCP_h
#define _GCP_h
/*
    Heat Theory:
        1) Turn on machine
        2) Heat water to brew temperature, or steam temperature if that switch is on
        3) Turn on lights when at specific temperatures
    Only input should be steam switch, which moves PID temp to 155 C
    For now leave pump switch manually on the coffee switch

*/

#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include <PID_v1.h>

using namespace std;

#define ON HIGH
#define OFF LOW

#define HEATER_PIN 13
#define STEAM_PIN 27

#define RREF 430
#define DEFAULT_BREW_TEMP 96.0
#define DEFAULT_STEAM_TEMP 135.0
#define DEFAULT_OFFSET -6.0
#define EMERGENCY_SHUTOFF_TEMP 150.0
#define MAX_OFFSET 15
#define MIN_OFFSET -15

#define CYCLE_TIME 2000
#define MAX_QUEUE_SIZE 120

enum mode{brew, steam};

struct Queue {
    int front, rear, capacity, count;
    String *queue;

    Queue(int c) {
        front = rear = 0;
        capacity = c;
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
private:
    Adafruit_MAX31865 tempProbe = Adafruit_MAX31865(A5);
    mode currentMode;

    void init(double, double, double);
    void parseQueue(ulong);

    double actualTemp;
    double tempOffset = DEFAULT_OFFSET;
    double targetTemp = DEFAULT_BREW_TEMP;
    double targetSteamTemp = DEFAULT_STEAM_TEMP;
    
    double maxBrewTemp = 100.0;
    double minBrewTemp = 85.0;

    double maxSteamTemp = 140.0;
    double minSteamTemp = 120.0;

    double brew_output;
    double steam_output;

    ulong cycleStartTime;

    PID brewTempManager = PID(&actualTemp, &brew_output, &targetTemp, 125, 150, 50, P_ON_M, DIRECT);
    PID steamTempManager = PID(&actualTemp, &steam_output, &targetSteamTemp, 125, 150, 50, P_ON_M, DIRECT);

    String outputString;
    Queue outputQueue = Queue(MAX_QUEUE_SIZE);

public:
    void start();
    mode getCurrentMode();
    void setMode(mode);
    void incrementTemp();
    void decrementTemp();
    double getTargetTemp();
    double getTargetSteamTemp();
    double getActualTemp(); //returns current temperature value
    double getTempOffset();
    String getOutput();
    String getTunings();
    void setTargetTemp(double); // Sets temperature to control to (setpoint)
    void setTargetSteamTemp(double);
    void setTempOffset(double);
    void setTunings(double, double, double);
    void refresh(ulong);
};
#endif