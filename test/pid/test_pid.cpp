#include "header.h"
#include "unity.h"

PID pid;
#define TARGET_TEMP 95.0
#define BOILER_POWER 1370

void test_belowTargetTemp() {
    double output = pid.compute(25.0, TARGET_TEMP);
    TEST_ASSERT_GREATER_OR_EQUAL(0.0, output);
}

void test_atTargetTemp() {
    double output = pid.compute(95.0, TARGET_TEMP);
    TEST_ASSERT_EQUAL(0.0, output);
}

void test_aboveTargetTemp(){
    double output = pid.compute(100.0, TARGET_TEMP);
    TEST_ASSERT_EQUAL(0.0, output);
}

//Pulls simulation example from https://github.com/hekike/liquid-pid
void test_isProperlyHeating() {
    double output;
    double actualTemp = 25.0;
    double ambientTemp = 25.0;
    double heatResistance = .606;
    double cWater = 4181.6;  // Heat Capacity of water at 25C https://www.engineeringtoolbox.com/specific-heat-capacity-water-d_660.html
    double m = .1; // Boiler capacity of GCP in kg https://www.gaggia-na.com/products/gaggia-classic-pro#specs
    double dt = 0.25; // Simulated step time

    for(int k = 0; k < (1800 / dt); k++) { // 30 minute simulated time
        int t = k * dt;
        output = pid.compute(TARGET_TEMP, actualTemp);
        
        double outputWattage = output * BOILER_POWER;

        actualTemp += ((outputWattage - (actualTemp - ambientTemp) / heatResistance) * dt / (cWater * m));

        if(t % 12 == 0) { // Print every 5th step interval
            Serial.printf("\nTime: %0.1f Temp: %f PWM %%: %f Power: %f", t / 60.0, actualTemp, output * 100.0, outputWattage);
        }
    }
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_belowTargetTemp);
    RUN_TEST(test_atTargetTemp);
    RUN_TEST(test_aboveTargetTemp);
    RUN_TEST(test_isProperlyHeating);
}

void loop () {
    UNITY_END();
}