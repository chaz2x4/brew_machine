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

#define HEATER_PIN 13
#define STEAM_PIN 27
#define RREF 430

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

    void init(double, double, double);
    void parseQueue(ulong);

    double actualTemp;
    double tempOffset = -6.0;
    double targetTemp = 98.0;
    double targetSteamTemp = 150.0;

    const double emergencyShutoffTemp = 165.0;
    const double maxBrewTemp = 100.0;
    const double minBrewTemp = 85.0;
    const double maxSteamTemp = 160.0;
    const double minSteamTemp = 140.0;
    const double maxOffset = 15;
    const double minOffset = -15;
    const int websiteQueueSize = 120;

    double brew_output;
    double steam_output;

    ulong cycleStartTime;
    const ulong cycleTime = 2000;
   
    PID brewTempManager = PID(&actualTemp, &brew_output, &targetTemp, 125, 150, 50, P_ON_M, DIRECT);
    PID steamTempManager = PID(&actualTemp, &steam_output, &targetSteamTemp, 125, 150, 50, P_ON_M, DIRECT);

    String outputString;
    Queue outputQueue = Queue(websiteQueueSize);
public:
    void start();
    void incrementTemp(String);
    void decrementTemp(String);
    double getTargetTemp();
    double getTargetSteamTemp();
    double getActualTemp(); //returns current temperature value
    double getTempOffset();
    String getOutput();
    String getTunings(String);
    void setTargetTemp(double); // Sets temperature to control to (setpoint)
    void setTargetSteamTemp(double);
    void setTempOffset(double);
    void setTunings(String, double, double, double);
    void refresh(ulong);
};
#endif