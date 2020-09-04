#include "header.h"
#include "unity.h"

GCP bm;

void test_constructor(){
    GCP bm(92.0);
    TEST_ASSERT_EQUAL_FLOAT(92.0, bm.getTargetTemp());
}

void test_isBrewReady () {
    TEST_ASSERT_FALSE(bm.isBrewReady());
}

void test_isSteamReady () {
    TEST_ASSERT_FALSE(bm.isSteamReady());
}

void test_isPowerOn(){
    TEST_ASSERT_TRUE(bm.isPowerOn());
}

void test_defaultTargetTemp() {
    TEST_ASSERT_EQUAL_FLOAT(DEFAULT_TARGET_TEMP, bm.getTargetTemp());
}

void test_setTargetTemp() {
    bm.setTargetTemp(92.0);
    TEST_ASSERT_EQUAL_FLOAT(92.0, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_incrementTemp(){
    float targetTemp = bm.getTargetTemp() + 0.5;
    bm.incrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(targetTemp, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_decrementTemp(){
    float targetTemp = bm.getTargetTemp() - 0.5;
    bm.decrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(targetTemp, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_maxBrewTemp(){
    bm.setTargetTemp(MAX_BREW_TEMP + 10);
    TEST_ASSERT_EQUAL_FLOAT(MAX_BREW_TEMP, bm.getTargetTemp());
    bm.incrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(MAX_BREW_TEMP, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_minBrewTemp(){
    bm.setTargetTemp(MIN_BREW_TEMP - 10);
    TEST_ASSERT_EQUAL_FLOAT(MIN_BREW_TEMP, bm.getTargetTemp());
    bm.decrementTemp();
    TEST_ASSERT_EQUAL_FLOAT(MIN_BREW_TEMP, bm.getTargetTemp());
    bm.setTargetTemp(DEFAULT_TARGET_TEMP);
}

void test_maxSteamTemp(){
    bm.setTargetTemp(165.0, MIN_STEAM_TEMP, MAX_STEAM_TEMP);
    TEST_ASSERT_EQUAL_FLOAT(MAX_STEAM_TEMP, bm.getTargetTemp());
}

void test_minSteamTemp(){
    bm.setTargetTemp(130.0, MIN_STEAM_TEMP, MAX_STEAM_TEMP);
    TEST_ASSERT_EQUAL_FLOAT(MIN_STEAM_TEMP, bm.getTargetTemp());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_constructor);

    RUN_TEST(test_isBrewReady);
    RUN_TEST(test_isSteamReady);
    RUN_TEST(test_isPowerOn);

    RUN_TEST(test_defaultTargetTemp);
    RUN_TEST(test_setTargetTemp);

    RUN_TEST(test_incrementTemp);
    RUN_TEST(test_decrementTemp);

    RUN_TEST(test_minBrewTemp);
    RUN_TEST(test_maxBrewTemp);

    RUN_TEST(test_minSteamTemp);
    RUN_TEST(test_maxSteamTemp);
}

void loop () {
    UNITY_END();
}