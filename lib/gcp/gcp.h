// GCP.h

#ifndef _GCP_h
#define _GCP_h

#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include <QuickPID.h>
#include <sTune.h>
#include <ArduinoJson.h>

/* PINS */
#define HEATER_PIN 27
#define STEAM_PIN 12
#define THERMOPROBE_PIN A5
#define THERMOPROBE_READY_PIN 33

/* EEPROM ADDRESSES */
#define BREW_TEMP_ADDRESS 0
#define STEAM_TEMP_ADDRESS 8
#define OFFSET_ADDRESS 16
#define SCALE_ADDRESS 24
#define TUNING_ADDRESS 32

/* HARDWARE VERSIONS */
#define ACS_VERSION ACS712_30A
#define RREF 430

typedef enum {
    BREW,
    STEAM,
    OFFSET
} TempMode;

class GCP {
public:
    GCP();
    ~GCP();
    void start();
    void setTargetTemp(TempMode, float);
    void incrementTemp(String);
    void decrementTemp(String);
    void incrementTemp(TempMode);
    void decrementTemp(TempMode);
    float getTargetTemp(TempMode);
    float getActualTemp();
    float getCurrentTemp();
    const char* getScale();
    String getOutput();
    String getLastOutput();
    String getTunings(String);
    String getTunings(TempMode);
    void setTunings(String, float, float, float);
    void setTunings(TempMode, float, float, float);
    void changeScale(String);
    void refresh(ulong);
private:
    Adafruit_MAX31865 tempProbe;
    const float kEmergencyShutoffTemp;
    const float kMaxBrewTemp;
    const float kMinBrewTemp;
    const float kMaxSteamTemp;
    const float kMinSteamTemp;
    const float kMaxOffset;
    const float kMinOffset;
    const int kWindowSize;
    const int kLogInterval;
    const int kOutputStep;
    const int kTuneTime;
    const int kSamples;
    const int kSettleTime;
    typedef enum {C, F} TempScale;

    float current_temp;
    float temp_offset;
    float target_brew_temp;
    float target_steam_temp;

    float brew_output;
    float steam_output;

    float brew_kp;
    float brew_ki;
    float brew_kd;

    float steam_kp;
    float steam_ki;
    float steam_kd;

    bool tuning_complete;
    bool currently_pwm;
    sTune brewTuner;
    sTune steamTuner;

    QuickPID brewTempManager;
    QuickPID steamTempManager;

    ulong log_start_time;
    ulong window_start_time;

    void loadParameters();
    TempMode modeToEnum(String);
    float PWM(TempMode, QuickPID*, sTune*, int, float, float);

    struct Queue {
        int front, rear, capacity, count;

        TempScale scale;
        ulong *times;
        float *temps;
        float *outputs[2];
        float *targets[3];

        Queue(int c, TempScale s) {
            scale = s;
            front = rear = 0;
            capacity = c;
            times = new ulong[capacity];
            temps = new float[capacity];
            for(int i=0;i<2;i++){
                outputs[i] = new float[capacity];
            }
            for(int i=0;i<3;i++) {
                targets[i] = new float[capacity];
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

        float sanitize(float t) {
            return sanitize(t, false, false);
        }

        float sanitize(float t, bool getUnrounded) {
            return sanitize(t, getUnrounded, false);
        }

        float sanitize(float t, bool getUnrounded, bool getOffset) {
            float result;
            if(scale == F) {
                result = t * 9 / 5;
                if(!getOffset) result += 32;
            }
            else result = t;
    
            if(getUnrounded) return result;
            else {
                if(scale == C) {
                    return round(result * 2.0) / 2.0;
                }
                else return round(result);
            }
        }

        void push(
            ulong time, 
            float temp, 
            float brew_output, 
            float steam_output, 
            float brew_target, 
            float steam_target, 
            float offset_target
        ){
            if(count == capacity) shift();
            if(rear == capacity - 1) rear = -1;
            if(count == 0) {
                times[0] = time;
                temps[0] = temp;
                outputs[BREW][0] = brew_output;
                outputs[STEAM][0] = steam_output;
                targets[BREW][0] = brew_target;
                targets[STEAM][0] = steam_target;
                targets[OFFSET][0] = offset_target;
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

        void shift() {
            front++;
            if(front == capacity) front = 0;
            count--;
        }

        int size() {
            return count;
        }

        String getAll() {
            DynamicJsonDocument output(24576);
            for(int i = 0; i<count; i++) {
                JsonObject results = output.createNestedObject();
                results["time"] = times[i];
                results["temperature"] = sanitize(temps[i], 1);
                results["scale"] = scale == F ? "F" : "C";

                JsonObject json_outputs = results.createNestedObject("outputs");
                json_outputs["brew"] = outputs[BREW][i];
                json_outputs["steam"] = outputs[STEAM][i];

                JsonObject json_targets = results.createNestedObject("targets");
                json_targets["brew"] = sanitize(targets[BREW][i]);
                json_targets["steam"] = sanitize(targets[STEAM][i]);
                json_targets["offset"] = sanitize(targets[OFFSET][i], false, true);
            }
            String outputString;
            serializeJson(output, outputString);
            return outputString;
        }

        String getLast(){
            DynamicJsonDocument output(192);
            int i = rear;
            output["time"] = times[i];
            output["temperature"] = sanitize(temps[i], 1);
            output["scale"] = scale == F ? "F" : "C";

            JsonObject json_outputs = output.createNestedObject("outputs");
            json_outputs["brew"] = outputs[BREW][i];
            json_outputs["steam"] = outputs[STEAM][i];

            JsonObject json_targets = output.createNestedObject("targets");
            json_targets["brew"] = sanitize(targets[BREW][i]);
            json_targets["steam"] = sanitize(targets[STEAM][i]);
            json_targets["offset"] = sanitize(targets[OFFSET][i], false, true);

            String outputString;
            serializeJson(output, outputString);
            return outputString;
        }
    };
    Queue outputQueue;
};
#endif
